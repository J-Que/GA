import os
import json
os.chdir(os.path.dirname(__file__) + "/../../")


class Report():
    def __init__(self, config):
        self.params = config.params
        self.cvrp = config.attrs["CVRP"]
        self.ga = config.attrs["GA"]
        self.cpu = config.attrs["CPU"]
        self.data = {"problem":self.params["problem"], "cost":None, "best known cost":None, "performance gap":None, "time":None, \
            "seconds per generation":None, "mutation rate":self.params["mutation rate"], "generaitons":self.params["generations"], "solution":None}


    def __read(self):
        with open("log.out", "r") as f:
            lines = f.readlines()
            self.data["cost"]                   = lines[0][-1]
            self.data["best known cost"]        = lines[1][-1]
            self.data["performance gap"]        = lines[2][-1]
            self.data["time"]                   = lines[3][-1]
            self.data["seconds per generation"] = lines[4][-1]
            self.data["solution"]               = lines[5][-1].split()


    def save(self):
        self.__read()
        with open("results/" + self.cvrp["set"] + "/" + self.problem + "_results.json", "w") as f:
            json.dump(self.data, f, indent=4)


    def header(self):
        print("\n" + "_" * 132)
        print("\nCVRP: ", self.params["problem"])
        print("-" * 132)
        n, c, b, o = self.cvrp["N"], self.cvrp["capacity"], self.cvrp["best"], bool(self.cvrp["optimal"])
        print(f"Nodes:  {n:<26} Capacity:  {c:<23} Best Known:  {b:<21} Optimal:  {o:<20}")
        
        print("\nGenetic Algorithm with Two-Opt Optimization")
        print("-" * 132)
        bits, d, x, y, m = self.ga["encoded bits"], self.ga["demand bits"], self.ga["x bits"], self.ga["y bits"], self.params["mutation rate"]
        print(f"Encoded Bits:  {bits:<19} X Bits:  {x:<25} Y Bits:  {y:<25} Demand Bits:  {d:<16}")
        print(f"Crossover:  One-Point              Mutation:  Inverse                 Selection:  Elitist                Mutation Rate:  {m:<14}")
        g, m, s = self.params["generations"], self.cpu["M"], (bits * self.cpu["M"] * n)/8_000_000
        print(f"Max Generations:  {g:<16} Population Size:  {m:<16} Memory MB:  {s:<18}")

        t, q = self.cpu["number of threads"], self.cpu["nodes per thread"]
        print(f"Threads:  {t:<24} Individuals/Thread:  {q:<10}")

        gen, cost, gap, t, spg = "Ge", "Cost", "Gap", "Time", "s/Gen"
        print(f"\n{gen:<20} {cost:<20} {gap:<20} {t:<20} {spg:<20}")
        print("-" * 132)


    def footer(self):
        print("\n" + "-" * 150)
        print("Problem:",                self.data["problem"])
        print("Cost:",                   self.data["cost"])
        print("Performance Gap:",        self.data["performance gap"])
        print("Generations:",            self.data["generations"])
        print("Total Execution Time:",   self.data["time"])
        print("Seconds per Genration:",  self.data["seconds per generation"])
        print(("-" * 150) + "\n")
