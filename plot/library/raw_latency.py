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
print sys.argv
filedict = dict()
if makeCache:    
    suffixes = ["dat"]
    filenames=sys.argv
    filedict = organizeFilenames(filenames, suffixes)
    fullDB = populateDataBases(filedict)


color=iter(cm.rainbow(np.linspace(0,1,100)))
c=next(color)
plt.rcParams.update({'font.size': 20})

tmpcolor = ['b','r','c','k', 'm', 'g']
cindex =0

plt.rcParams.update({'font.size': 20})

figure(num=None, figsize=(15, 15), dpi=80, facecolor='w', edgecolor='k')
cindex = 0
maxlat = 0
minlat = 999999


i=7
for name in filedict:
            
            sname = name.split("/")
            subname = sname[len(sname)-1]
            leftname = subname.split("_")
            finalname = leftname[0]
            
            print(finalname)
            lat = filedict[name]['dat']['Latency']
            time = filedict[name]['dat']['Time']
            mslat = NanoToMicroSeconds(lat)

            x, y = gen_cdf(mslat,1000)
            #plt.plot(x,y,linewidth=i,label=finalname)
            #plt.plot(x,y,linewidth=i,label=name)
            plt.plot(x,y,linewidth=i,label=sname[0])

            print(x)

            i=i-2
            cindex = cindex + 1

plt.grid('on')

plt.xlabel("Resposne Latency (us)", fontweight='bold')

#lgd = plt.legend(ncol=3,loc="lower center",bbox_to_anchor=(0.50,-0.20))
plt.legend()
#plt.title("Static interval 25us per request, 10us delay",fontweight='bold')
#plt.title("Dynamic interval 25us per request, 10us delay +/- 25%",fontweight='bold')
plt.title("Dynamic interval 25us per request, 10us delay +/- 50%",fontweight='bold')
plt.tight_layout(rect=(0,0.1,1,1))
plt.savefig("Replica.pdf")

