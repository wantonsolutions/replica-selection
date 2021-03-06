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

from scipy.stats import sem

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
def setUniformParams():
    plt.rc('axes.formatter', useoffset=False)
    plt.legend()
    plt.grid(True, which="both", ls=":", color='0.20')
    plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
    plt.ylabel("Packet Redirections", fontweight='bold')
    plt.tight_layout()


sys.argv.pop(0)

print sys.argv


color_comet=sns.xkcd_rgb["medium blue"]
colors=dict()
colors["random"]=sns.xkcd_rgb["medium blue"]
colors["single"]=sns.xkcd_rgb["pale red"]
colors["minimum"]=sns.xkcd_rgb["medium green"]


plt.rcParams.update({'font.size': 20})
plt.figure(figsize=(20,10),dpi=20)

linetype = ['-',':','--','-.']


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
            map_results[selection+"_x"]=[]# divide by 1000
            map_results[selection+"_NodeRedirections"]=[]
            map_results[selection+"_NodeTotal"]=[]
            map_results[selection+"_EdgeRedirections"]=[]
            map_results[selection+"_EdgeTotal"]=[]
            map_results[selection+"_AggRedirections"]=[]
            map_results[selection+"_AggTotal"]=[]
            map_results[selection+"_CoreRedirections"]=[]
            map_results[selection+"_CoreTotal"]=[]
        #map_results[selection+"_x"].append(int(row[0]) / 1000) # divide by 1000
        map_results[selection+"_x"].append(int(row[0])) # divide by 1000
        map_results[selection+"_NodeRedirections"].append(float(row[3]))
        map_results[selection+"_NodeTotal"].append(float(row[4]))
        map_results[selection+"_EdgeRedirections"].append(float(row[5]))
        map_results[selection+"_EdgeTotal"].append(float(row[6]))
        map_results[selection+"_AggRedirections"].append(float(row[7]))
        map_results[selection+"_AggTotal"].append(float(row[8]))
        map_results[selection+"_CoreRedirections"].append(float(row[9]))
        map_results[selection+"_CoreTotal"].append(float(row[10]))

    

    for selection in selections:
        map_results[selection+"_NodeRedirections"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_NodeRedirections"]))]
        map_results[selection+"_NodeTotal"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_NodeTotal"]))]
        map_results[selection+"_EdgeRedirections"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_EdgeRedirections"]))]
        map_results[selection+"_EdgeTotal"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_EdgeTotal"]))]
        map_results[selection+"_AggRedirections"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_AggRedirections"]))]
        map_results[selection+"_AggTotal"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_AggTotal"]))]
        map_results[selection+"_CoreRedirections"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_CoreRedirections"]))]
        map_results[selection+"_CoreTotal"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_CoreTotal"]))]

    for selection in selections:
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_NodeRedirections"],label=selection+"_NodeRedirections",color=colors[selection],linestyle=linetype[0], marker=".")
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_EdgeRedirections"],label=selection+"_EdgeRedirections",color=colors[selection],linestyle=linetype[1], marker=".")
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_AggRedirections"],label=selection+"_AggRedirections",color=colors[selection],linestyle=linetype[2], marker=".")
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_CoreRedirections"],label=selection+"_CoreRedirections",color=colors[selection],linestyle=linetype[3], marker=".")

    #plt.tight_layout(rect=(0,0.1,1,1))
    setUniformParams()
    plt.savefig("Switch_Redirections_All.pdf")
    plt.clf()

    plt.rcParams.update({'font.size': 20})
    plt.figure(figsize=(20,10),dpi=20)
    for selection in selections:
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_CoreRedirections"],label=selection+"_CoreRedirections",color=colors[selection],linestyle=linetype[3], marker=".")
    setUniformParams()
    plt.savefig("Switch_Redirections_Core.pdf")
    plt.clf()

    plt.rcParams.update({'font.size': 20})
    plt.figure(figsize=(20,10),dpi=20)
    for selection in selections:
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_AggRedirections"],label=selection+"_AggRedirections",color=colors[selection],linestyle=linetype[3], marker=".")
    setUniformParams()
    plt.savefig("Switch_Redirections_Agg.pdf")
    plt.clf()

    plt.rcParams.update({'font.size': 20})
    plt.figure(figsize=(20,10),dpi=20)
    for selection in selections:
        plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_EdgeRedirections"],label=selection+"_EdgeRedirections",color=colors[selection],linestyle=linetype[3], marker=".")
    setUniformParams()
    plt.savefig("Switch_Redirections_Edge.pdf")
    plt.clf()

    sumResults=dict()
    for selection in selections:
        summerR=[]
        summerR.append(map_results[selection+"_NodeRedirections"])
        summerR.append(map_results[selection+"_EdgeRedirections"])
        summerR.append(map_results[selection+"_AggRedirections"])
        summerR.append(map_results[selection+"_CoreRedirections"])
        redirects=[sum(x) for x in zip(*summerR)]
        print(redirects)        

        summerT=[]
        summerT.append(map_results[selection+"_NodeTotal"])
        summerT.append(map_results[selection+"_EdgeTotal"])
        summerT.append(map_results[selection+"_AggTotal"])
        summerT.append(map_results[selection+"_CoreTotal"])
        totals=[sum(x) for x in zip(*summerT)]
        print(totals)

        diff=[(a/b)*100.0 for a, b in zip(redirects,totals)]

        plt.plot(sorted(map_results[selection+"_x"]),diff,label=selection,color=colors[selection],linestyle=linetype[1], marker=".")

    setUniformParams()
    plt.ylabel("Percentage of Packets Redirected")
    plt.savefig("Percent_Switch_Redirects.pdf")
    plt.clf()

