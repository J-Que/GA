import os
import json
import multiprocessing
import numpy as np
os.chdir(os.path.dirname(__file__) + "/../../")


class Manager():
    def __init__(self, argv):
        # the arguements passed in
        self.argv = argv

        # arguements that may be passed in
        self.options = {"g":"generations", "l":"population mulitplier", "m":"mutation rate", "p":"problem"}
        
        # read in parameters
        with open("params.json", "r") as f: 
            self.params = json.load(f)

        # attributes of the algorithm
        self.attrs = {"CVRP":{}, "GA":{}, "CPU":{}, "GPU":{}, "RL":{}}


    # search the arguements and overwrite any given arguement
    def __overwrite(self):
        if len(self.argv) > 1:
            for i, arg in enumerate(self.argv):
                for option in self.options:
                    if arg == "+" + option:
                        self.params[self.options[option]] = self.argv[i + 1]


    # get the attributes for the cvrp
    def __cvrp(self, df):
        self.attrs["CVRP"].update({"set":     df["Set"]})
        self.attrs["CVRP"].update({"capacity":df["Capacity"]})
        self.attrs["CVRP"].update({"best":    df["Best"]})
        self.attrs["CVRP"].update({"optimal": df["Optimal"]})
        self.attrs["CVRP"].update({"depot":   df["Nodes"][0][-1]})
        self.attrs["CVRP"].update({"N":       len(df["Nodes"])})


    # get the attributes for the ga
    def __ga(self, df):
        self.attrs["GA"].update({"encoded bits":df["Encoded Bits"]})
        self.attrs["GA"].update({"demand bits": df["Demand Bits"]})
        self.attrs["GA"].update({"x bits":      df["X Bits"]})
        self.attrs["GA"].update({"y bits":      df["Y Bits"]})


    # get the attributes for the multi-threading
    def __cpu(self, df):
        threads = multiprocessing.cpu_count()
        M = (2 * threads) * ((self.params["population multiplier"] * self.attrs["CVRP"]["N"]) // (2 * threads))
        self.attrs["CPU"].update({"M":M, "number of threads":threads, "nodes per thread":M//threads})


    # read in the problem and get the rest of the attributes
    def __attributes(self):
        with open("data/test/" + self.params["problem"].split("-")[0] + "/" + self.params["problem"] + ".json", "r") as f: df = json.load(f)
        self.__cvrp(df)
        self.__ga(df)
        self.__cpu(df)
        self.nodes = df["Nodes"]


    def __write(self, arr):
        line = "\n    "
        for node in arr: line += str(node) + ", "
        return line[:-2] + "],"


    def __save(self):
        lines = [arr[i] for i in range(len)]



        # save the population
        with open("population.cpp", "w") as f:

            M, N = str(self.attrs["CPU"]["M"]), str(self.attrs["CVRP"]["N"])
            f.write("int[" + M + "][" + N + "] population = [")
            for arr in self.population[:-1]:
                f.write("\n    [")
                for node in arr[:-1]: f.write(str(node) + ",  ")
                f.write(str(arr[-1]) + "],")
            for node in self.population[-1]: f.write(str(node) + ",  ")
            f.write(str(self.population[-1][-1]) + "],\n];")


    # create a population for the problem
    def __genZ(self):
        # create a random population
        self.population = np.array(self.nodes[1:])[:,-1]
        self.population = np.tile(self.population, (5, 1))
        for arr in self.population: np.random.shuffle(arr)                


    def __meta(self): pass


    def __compile(self): pass


    # configure the algorithm file according to the parameters and the problem attributes
    def config(self):
        self.__overwrite()
        self.__attributes()
        self.__genZ()
        self.__meta()
        self.__compile()
        