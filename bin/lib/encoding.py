import numpy as np


class Encoder():
    def __init__(self, nodes, demand, file, xBits=11, yBits=11, demandBits=9, depotBits=2):
        self.coordinates = nodes
        self.demand = demand
        self.xBits = xBits
        self.yBits = yBits
        self.demandBits = demandBits
        self.depotBits = depotBits
        self.N = len(self.demand)
        self.file = file
        self.nodes = np.array([], dtype=np.int64)


    # normalize the coordinates
    def normalize(self):
        # get the x and y
        self.coordinates = np.array(self.coordinates)[:, 1:3]

        # subtract the minimum value to get a range from 0 to max
        self.coordinates[:, 0] = self.coordinates[:, 0] - np.min(self.coordinates[:, 0])
        self.coordinates[:, 1] = self.coordinates[:, 1] - np.min(self.coordinates[:, 1])

        # round the coordinates
        self.coordinates = np.round(self.coordinates)

        # change to integer
        self.coordinates = self.coordinates.astype('int64')


    # validate the demand
    def validate(self):
        # round the demand up
        self.demand = np.ceil(self.demand)

        # determine if there is any negative demand
        if np.min(self.demand) < 0: print("NEGATIVE DEMAND!")

        # change to integer
        self.demand = self.demand.astype('int64')


    # get the number of bits for each attribute
    def bit_sizes(self):
        maxX = int(np.max(self.coordinates, 0)[0])
        maxY = int(np.max(self.coordinates, 0)[1])
        maxD = int(np.max(self.demand))

        for i in range(64):
            if maxX < pow(2, i):
                self.xBits = i + 1
                break
        
        for i in range(64):
            if maxY < pow(2, i):
                self.yBits = i + 1
                break
        
        for i in range(64):
            if maxD < pow(2, i):
                self.demandBits = i + 1
                break

        self.bits = self.depotBits + self.demandBits + self.xBits + self.yBits
        #if self.bits > 32:print(self.f, self.bits,self.demandBits, self.xBits, self.yBits, self.depotBits)


    # encode a node
    def encode(self, n):
        y      = self.coordinates[n][1]
        x      = self.coordinates[n][0] << self.yBits
        demand = self.demand[n] << (self.xBits + self.yBits)
        self.nodes = np.append(self.nodes, demand + x + y)
   
        
    # adjust all nodes so that they may be encoded
    def encode_all(self):
        self.normalize()
        self.validate()
        self.bit_sizes()
        for n in range(self.N): self.encode(n)
        return self.nodes
