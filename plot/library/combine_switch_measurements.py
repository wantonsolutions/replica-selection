import numpy as np
from pylab import *
from matplotlib.pyplot import figure
from matplotlib.font_manager import FontProperties
import matplotlib.pyplot as plt
import matplotlib.colors as colors
import csv
import sys
import pickle

from lib import *


## The point of this graphing program is to combine the latency graphs from
#previous plots with queue information.  ## This libary should take pairs of
#files as inputs. Each pair should include a .dat for the latency and packet
#drop. Each file should be coupled by the prefixes that it has. Each file should
#also be indexable by its suffex so seperate graphs can be generated for
#individual runs. Further comparitive graphs can easily be generated between
#paris of runs. The first task then is to parse the input and make sure that
#each file pair matches a prefix list. Eventually this prefix list could be an
#entire database, but the seperation of concerns is good to start with.


sys.argv.pop(0)

makeCache = True
#plot the latencies of each individual measure

f=open("aggregate_switch.dat","w")


filenames=sys.argv
for filename in filenames:
    counter=0
    data=dict()
    names=[]
    with open(filename,'r') as csvfile:
        plots = csv.reader(csvfile, delimiter=',')
        counter=0
        for row in plots:
            #skip the first line
            if counter == 0:
                counter=1
                continue

            sname = filename.split("/")
            nameagg=""
            for field in sname[1:]:
                nameagg=nameagg+field+","

            rest=""
            for value in row:
                rest=rest+value+","
            
            f.write(nameagg+rest+"\n")
            break
f.close()