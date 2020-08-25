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
filedict = dict()

percentiles=[0.5 , 0.95, 0.99, 0.999, 0.99999]

f = open("aggregate.dat",'w')
if makeCache:    
    suffixes = ["dat"]
    filenames=sys.argv
    filedict = organizeFilenames(filenames, suffixes)
    fullDB = populateDataBases(filedict)

for name in filedict:
            
            sname = name.split("/")
            subname = sname[len(sname)-1]
            leftname = subname.split("_")
            finalname = leftname[0]

            nameagg=""
            for field in sname[1:]:
                nameagg=nameagg+field+","
            
            lat = filedict[name]['dat']['Latency']
            sojourn = filedict[name]['dat']['Sojourn']
            mslat = NanoToMicroSeconds(lat)
            mssoj = NanoToMicroSeconds(sojourn)

            cdf_granularity = 10000
            x, y = gen_cdf(mslat,cdf_granularity)
            sojx, sojy = gen_cdf(mssoj,cdf_granularity)
            #find 50
            pIndexes=[]
            for i in percentiles:
                index=0
                for j in y:
                    if j < i:
                        index=index+1
                    else:
                        break
                pIndexes.append(index)

            outputStr=nameagg
            for percentileIndex in pIndexes:
                outputStr = outputStr + str(x[percentileIndex]) + ","
            for percentileIndex in pIndexes:
                outputStr = outputStr + str(sojx[percentileIndex]) + ","
            outputStr = outputStr + "\n"

            #f.write(nameagg+str(x[x50]) + "," + str(x[x95]) + "," + str(x[x99]) + "," + str(sojx[x50]) + "," + str(sojx[x95]) + "," + str(sojx[x99])+"\n")
            f.write(outputStr)
            #plt.plot(x,y,linewidth=i,label=finalname)

f.close()