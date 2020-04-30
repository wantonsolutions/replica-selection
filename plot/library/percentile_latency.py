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
            time = filedict[name]['dat']['Time']
            mslat = NanoToMicroSeconds(lat)

            x, y = gen_cdf(mslat,1000)
            x50 = (len(x) / 100 ) * 50
            x95 = (len(x) / 100 ) * 95
            x99 = (len(x) / 100 ) * 99
            print nameagg+str(x[x50]) + "," + str(x[x95]) + "," + str(x[x99])
            #plt.plot(x,y,linewidth=i,label=finalname)