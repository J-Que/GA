import sys
import json
import numpy as np
sys.path.insert(0, 'lib')
import Encoder

def read():
    with open("data/data/" + sys.argv[1] + ".json", "r") as f: data = json.load(f)
    data.update({"M":, "Max Generations":2000, "N":len(data["Nodes"]), "Thread Nodes":, "Mutation"})

def init_population(data):
    encoder = Encoder.Encoder(data["Problem"])
    encoder.encode_all()
    nodes = encoder.nodes
    nodes = np.tile(nodes, data["M"]).T
    np.random.shuffle(nodes)
    return nodes.T

def meta_program(data, nodes): pass

def header(): pass

def compile(): pass

def main():
    data = read()
    nodes = init_population(data)
    meta_program(data, nodes)
    header()
    compile()    

if __name__ == "__main__":
    main()