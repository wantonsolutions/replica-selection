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

color=iter(cm.rainbow(np.linspace(0,1,100)))
c=next(color)
plt.rcParams.update({'font.size': 20})

colors = ['b','r','c','k', 'm', 'g']
linetype = ['-',':','--']
cindex =0

plt.rcParams.update({'font.size': 20})

figure(num=None, figsize=(15, 15), dpi=80, facecolor='w', edgecolor='k')

filename=sys.argv[0]
print filename

map_results=dict()
selections=dict()

with open(filename,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        selection=row[1]
        if not selection in selections:
            selections[selection]=True
            map_results[selection+"_50"]=[]
            map_results[selection+"_95"]=[]
            map_results[selection+"_99"]=[]
            map_results[selection+"_x"]=[]
        #map_results[selection+"_x"].append(int(row[0]) / 1000) # divide by 1000
        map_results[selection+"_x"].append(int(row[0])) # divide by 1000
        map_results[selection+"_50"].append(float(row[3]))
        map_results[selection+"_95"].append(float(row[4]))
        map_results[selection+"_99"].append(float(row[5]))

    

    for selection in selections:
        map_results[selection+"_50"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_50"]))]
        map_results[selection+"_95"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_95"]))]
        map_results[selection+"_99"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_99"]))]

    for selection in selections:
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_50"],label=selection+"_50",color=colors[cindex],linestyle=linetype[0], marker="x")
        #plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_95"],label=selection+"_95",color=colors[cindex],linestyle=linetype[1], marker="x")
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_99"],label=selection+"_99",color=colors[cindex],linestyle=linetype[2], marker="x")
        cindex=cindex+1



plt.grid('on')

#ax = plt.gca()
#ax.ticklabel_format(useOffset=False)

plt.xscale("log")
plt.rc('axes.formatter', useoffset=False)
#lgd = plt.legend(ncol=3,loc="lower center",bbox_to_anchor=(0.50,-0.20))
plt.legend()
#plt.xlim(left=0.9,right=200)
#plt.ylim(top=2500,bottom=-50)
#plt.xlabel("Interval between requests (us)", fontweight='bold')
plt.xlabel("Packet Size (bytes)", fontweight='bold')
plt.ylabel("Response latency (us)", fontweight='bold')
#plt.title("Static request interval response latency",fontweight='bold')
plt.title("Uniform Requests 25us Variable Packet Sizes",fontweight='bold')
plt.tight_layout(rect=(0,0.1,1,1))
plt.savefig("Agg_Latency.pdf")

