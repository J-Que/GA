import os
import sys
import subprocess
os.chdir(os.path.dirname(__file__) + "/../../")
sys.path.insert(0, 'bin/scripts')
sys.path.insert(0, 'bin/lib')
import config
import report


def main():
    process = subprocess.Popen("pwd", stdout=subprocess.PIPE)
    return process.communicate()


if __name__ == "__main__":
    
    # configure the file
    config = config.Manager(sys.argv)
    config.config()

    # print a report header
    myReport = report.Report(config)
    myReport.header()

    # run the algorithm
    #soutput, error = main()

    # report on the results
    #report.save()
    #report.footer()
