import os
import sys
import json
import numpy as np
os.chdir(os.path.dirname(__file__) + '/../../')
sys.path.insert(0,'bin/lib/')
import encoding


sets = {"A":"Set A", "B":"Set B", "CMT":"Christofides, Mingozzi and Toth", "E":"Set E", "F":"Set F", "Golden":"Golden et al.", "Li":"Li et al.", "M":"Set M", "P":"Set P", "tai":"Rochat and Tailard", "X":"Uchoa et al.", "XXL":"Arnold, Genreau, Sorensen"}


def getNodes(index, lines, data):
    # get the indices of the lines containing the nodes and demand
    nodeStart = index + 1
    nodeEnd = nodeStart + data["Dimension"]
    demandStart = nodeEnd + 1
    demandEnd = demandStart + data["Dimension"]

    # add the node info
    nodes, demand = [], []
    index = -1
    # get the demand
    for n in range(demandStart, demandEnd):
        demand.append(float(lines[n].split()[-1]))

    # if the depot is at the end then place it at the begining
    if demand[-1] == 0:
        if demand[0] != 0:
            depot = demand.pop(-1)
            demand.insert(0, depot)
            depot = lines.pop(nodeEnd - 1)
            lines.insert(nodeStart, depot)

    # get the remaining info
    for n in range(nodeStart, nodeEnd):
        node = lines[n].split()
        nodes.append([index, float(node[1]), float(node[2]), 0, 0])
        index += 1

    # seperate the nodes and depot
    depot = nodes[0]
    return nodes, demand, depot


def optimality(dir, file):
    with open("data/raw/" + dir + "/" + file, "r") as f: contents = f.readlines()
    if dir in ["Golden", "Li", "XXL"]:          contents.insert(2, 'OPTIMAL : False\n')
    elif dir == "X" and file[3] in ["1", "2"]:  contents.insert(2, 'OPTIMAL : True\n')
    elif dir == "X":                            contents.insert(2, 'OPTIMAL : False\n')
    else:                                       contents.insert(2, 'OPTIMAL : True\n')
    with open("data/raw/" + dir + "/" + file, "w") as f: f.write("".join(contents))


def boolOptimality(str):
    if str[0] == 'T':   return True
    else:               return False


def getData(dir, file):
    data = {"Problem":file[:-4], "Set":sets[dir], "Capacity":None, "Dimension":None, "Best":None, "Optimal":None, "Encoded Bits":None, "X Bits":None, "Y Bits":None, "Demand Bits":None, "Index Bits":None, "Depot Bits":None, "Depot":None, "Nodes":None}
    with open("data/raw/" + dir + "/" + file, "r") as f:
        lines = f.readlines()
        for index, line in enumerate(lines):
            if line[0:3] == "COM":
                if len(line.split("value:")) == 2:  data["Best"]      = float(line.split("value:")[1][:-2])
                elif len(line.split()) == 3:        data["Best"]      = float(line.split()[-1])
            elif line[0:3] == "OPT":                data["Optimal"]   = boolOptimality(line.split(" : ")[-1])
            elif line[0:3] == "DIM":                data["Dimension"] = int(line.split(" : ")[-1])
            elif line[0:3] == "CAP":                data["Capacity"]  = int(line.split(" : ")[-1])
            elif line[0:3] == "NOD":                
                nodes, demand, depot = getNodes(index, lines, data)
                data["Depot"] = [depot[1], depot[2]]
                break

    return data, nodes, demand


def main(dir, file):
    # adjust optimality field

    # get the data from the file
    data, nodes, demand = getData(dir, file)

    # encode the nodes
    encoder = encoding.Encoder(nodes, demand, file, depotBits=1)
    encoder.encode_all()

    # get the encoding data
    data["Encoded Bits"] = encoder.bits
    data["X Bits"]       = encoder.xBits
    data["Y Bits"]       = encoder.yBits
    data["Demand Bits"]  = encoder.demandBits
    data["Index Bits"]   = encoder.indexBits
    data["Depot Bits"]   = encoder.depotBits

    for i in data:
        if data[i] == None:
            if i != "Nodes":
                print("Missing values", file, i)
                exit()

    # combine the node data
    for n in range(data["Dimension"]):
        nodes[n][-2] = demand[n]
        nodes[n][-1] = encoder.nodes[n] # add the enocded nodes

    try:    data["Nodes"] = np.array(nodes, dtype=np.int32)[1:].tolist()
    except: data["Nodes"] = np.array(nodes, dtype=np.int64)[1:].tolist()
    #except: data["Nodes"] = np.array(nodes, dtype=np.longlong).tolist()

    if   dir == "tai"   : newName = "tai-"    + file[3:-4]
    elif dir == "CMT"   : newName = "CMT-"    + file[3:-4]
    elif dir == "Li"    : newName = "Li-"     + file[3:-4]
    elif dir == "Golden": newName = "Golden-" + file[7:-4]
    elif dir == "XXL"   : newName = "XXL-"    + file[:-4]
    else:                 newName = file[:-4]

    with open("data/test/" + dir + "/" + newName + ".json", "w") as f: json.dump(data, f, indent=4)
    print(file, "finished")


if __name__ == "__main__":
    for dir in os.listdir("data/raw/"):
        try: os.mkdir("data/test/" + dir)
        except: pass
        for file in os.listdir("data/raw/" + dir):
            if file[-3:] == "vrp": main(dir, file)
