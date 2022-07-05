import os
import sys
import json
import numpy as np
os.chdir(os.path.dirname(__file__) + '/../../')
sys.path.insert(0, "bin/lib/")
import encoding

# keep track of the test results
class Report():
    def __init__(self):
        self.fails =[[set(), set(), set(), set()] for _ in range(6)]


    def print(self):
        h1, h2, h3, h4, f = "Total", "Tested vs Encoded", "Tested vs Saved", "Encoded vs Saved", self.fails
        print(f"                          {h1          :>4}", f"{h2          :>20}", f"{h3          :>20}", f"{h4          :>20}")
        print(f"failed files:             {len(f[0][0]):>4}", f"{len(f[0][1]):>20}", f"{len(f[0][2]):>20}", f"{len(f[0][3]):>20}")
        print(f"encoding failures:        {len(f[1][0]):>4}", f"{len(f[1][1]):>20}", f"{len(f[1][2]):>20}", f"{len(f[1][3]):>20}")
        print(f"decoded demand failures:  {len(f[2][0]):>4}", f"{len(f[2][1]):>20}", f"{len(f[2][2]):>20}", f"{len(f[2][3]):>20}")
        print(f"decoded x failures:       {len(f[3][0]):>4}", f"{len(f[3][1]):>20}", f"{len(f[3][2]):>20}", f"{len(f[3][3]):>20}")
        print(f"decoded y failures:       {len(f[4][0]):>4}", f"{len(f[4][1]):>20}", f"{len(f[4][2]):>20}", f"{len(f[4][3]):>20}")
        print(f"distance failures:        {len(f[4][0]):>4}", f"{len(f[4][1]):>20}", f"{len(f[4][2]):>20}", f"{len(f[4][3]):>20}")


# class for testing if the encoder class is working correctly
class encoderTest():
    def __init__(self, path, file, report):
        self.path = path
        self.file = file
        self.report = report

        # get the nodes from the problem file
        with open(path + "/" + file, "r") as f:
            data = json.load(f)
            self.nodes = np.array(data["Nodes"])
            self.c = data["Capacity"]
            self.n = data["Dimension"]
            self.saved = self.nodes[:, -1]

        # get the encoded nodes
        self.encoder = encoding.Encoder(self.nodes, self.nodes[:,3], file, depotBits=1)
        self.encoded = self.encoder.encode_all()
        
        # for test purposes
        self.xMin = np.min(self.nodes[:, 1])
        self.yMin = np.min(self.nodes[:, 2])
        self.xNorm = self.nodes[:, 1] - self.xMin
        self.yNorm = self.nodes[:, 2] - self.yMin


    # update the report
    def __update(self, test, section):
        self.report.fails[0][0].add(self.file)
        self.report.fails[0][section].add(self.file)
        self.report.fails[test][0].add(self.file)
        self.report.fails[test][section].add(self.file)


    # examine the comparison even closer
    def __examine(self, arr1, arr2, test, section, tolerance):
        for n in range(self.n):
            if abs(arr1[n] - arr2[n]) > tolerance:
                self.__update(test, section)


    # compare arrays
    def __compare(self, test, encoded, saved, t, tolerance):
        if np.sum(np.abs(test - encoded))    != 0: self.__examine(test,  encoded, t, 1, tolerance)
        if np.sum(np.abs(test    - saved))   != 0: self.__examine(test,    saved, t, 2, tolerance)
        if np.sum(np.abs(encoded - saved))   != 0: self.__examine(encoded, saved, t, 3, tolerance)


    # decode and get the demand and x and y
    def __decode(self, nodes):
        d = nodes >> (self.encoder.xBits + self.encoder.yBits)
        x = (nodes % pow(2, self.encoder.xBits + self.encoder.yBits)) // pow(2, self.encoder.yBits) + self.xMin
        y = (nodes % pow(2, self.encoder.yBits)) + self.yMin
        return d, x, y


    # get the distances between each subsequet node
    def __distance(self, x, y):
        xd, yd = x[1:] - x[:-1], y[1:] - y[:-1]
        dist2 = xd * xd + yd * yd
        return np.sqrt(dist2)


    # decode all the arrays
    def __calculate(self):
        self.td, self.tx, self.ty = self.nodes[:,3], self.nodes[:,1], self.nodes[:,2]
        self.ed, self.ex, self.ey = self.__decode(self.encoded)
        self.sd, self.sx, self.sy = self.__decode(self.saved)
        self.tDist = self.__distance(self.tx, self.ty)
        self.eDist = self.__distance(self.ex, self.ey)
        self.sDist = self.__distance(self.sx, self.sy)


    # test repeatabilty of encoding
    def encoding(self):
        # get the value of the encoded demand, x, and y and the encoded node
        d = self.nodes[:, 3] << (self.encoder.xBits + self.encoder.yBits)
        x = self.xNorm << self.encoder.yBits
        y = self.yNorm
        self.__compare(d + x + y, self.encoded, self.saved, 1, 4096)
        self.__calculate()


    # test if the demand, x, and y can be decoded correctly
    def decode_demand(self): self.__compare(self.td, self.ed, self.sd, 2, 0)
    def decode_x(self):      self.__compare(self.tx, self.ex, self.sx, 3, 1)
    def decode_y(self):      self.__compare(self.ty, self.ey, self.sy, 4, 1)
    def distance(self):      self.__compare(self.tDist, self.eDist, self.sDist, 5, 0)


# conduct the actual tests
def main(path, file, report):
    test = encoderTest(path, file, report)
    test.encoding()
    test.decode_demand()
    test.decode_x()
    test.decode_y()
    return test.report, test


if __name__ == '__main__':
    # keep track of any test failures and pass is report format
    report = Report()
    
    # iterate through all the files
    for dir in os.listdir('data/test/'):
        path = 'data/test/' + dir
        for file in os.listdir(path): 
            report, test = main(path, file, report)

    # print the report
    report.print()
