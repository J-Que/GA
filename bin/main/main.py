import os
import sys
import subprocess
os.chdir(os.path.dirname(__file__) + "/../../")
sys.path.insert(0, 'bin/scripts')
sys.path.insert(0, 'bin/lib')
import config as configs
import report


if __name__ == "__main__":
    
    # configure the file
    config = configs.Manager(sys.argv)
    config.configure()

    # print a report header
    myReport = report.Report(config)
    myReport.header()

    # run the algorithm
   # proc = subprocess.Popen(["g++", "bin/main/ex.cpp", "-o", "main"])
   # proc.wait()

    # report on the results
    #report.save()
    #report.footer()
