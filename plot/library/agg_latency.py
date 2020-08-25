import numpy as np
from pylab import *
from matplotlib.pyplot import figure
from matplotlib.font_manager import FontProperties
import matplotlib.pyplot as plt
import matplotlib.colors as colors
import csv
import sys
import pickle

import seaborn as sns

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

plt.rcParams.update({'font.size': 20})
plt.figure(figsize=(20,10),dpi=20)

color_comet=sns.xkcd_rgb["medium blue"]
colors=dict()
colors["random"]=sns.xkcd_rgb["medium blue"]
colors["single"]=sns.xkcd_rgb["pale red"]
colors["minimum"]=sns.xkcd_rgb["medium green"]

linetype = ['-',':','--']
cindex =0


suffixes=["_50", "_95", "_99","_99.9", "_99.99", "_soj_50", "_soj_95", "_soj_99","_soj_99.9", "_soj_99.99" ]
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
            map_results[selection+"_x"]=[]
            for suffix in suffixes:
                map_results[selection+suffix]=[]
        #map_results[selection+"_x"].append(int(row[0]) / 1000) # divide by 1000
        map_results[selection+"_x"].append(int(row[0])) # divide by 1000
        index = 3
        for suffix in suffixes:
            map_results[selection+suffix].append(float(row[index]))
            index=index+1

    #Sort the results
    for selection in selections:
        for suffix in suffixes:
            map_results[selection+suffix]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+suffix]))]
        map_results[selection+"_x"]=sorted(map_results[selection+"_x"]) ##Sort X here and no sooner


    for selection in selections:
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_50"],label=selection+"_50",color=colors[selection],linestyle=linetype[0], marker="x")
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_99"],label=selection+"_99",color=colors[selection],linestyle=linetype[2], marker="x")


plt.rc('axes.formatter', useoffset=False)
plt.legend()
plt.yscale("log")
plt.ylim(top=6200,bottom=10)
plt.grid(True, which="both", ls=":", color='0.20')
plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
plt.ylabel("Response latency (us)", fontweight='bold')
plt.tight_layout()
plt.savefig("Agg_Latency.pdf")

plt.clf()

plt.rcParams.update({'font.size': 20})
plt.figure(figsize=(20,10),dpi=20)

for selection in selections:
    x=sorted(map_results[selection+"_x"])
    network_50=[ a - b for a,b in zip(map_results[selection+"_50"],map_results[selection+"_soj_50"])]
    network_99=[ a - b for a,b in zip(map_results[selection+"_99"],map_results[selection+"_soj_99"])]
    plt.plot(x,network_50,label=selection+"_50",color=colors[selection],linestyle=linetype[0], marker="x")
    plt.plot(x,network_99,label=selection+"_99",color=colors[selection],linestyle=linetype[2], marker="x")


plt.rc('axes.formatter', useoffset=False)
plt.legend()
#plt.yscale("log")
#plt.ylim(top=6200,bottom=10)
plt.grid(True, which="both", ls=":", color='0.20')
plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
plt.ylabel("Network Transit Time (us)", fontweight='bold')
plt.tight_layout()
plt.savefig("Network_Latency.pdf")

