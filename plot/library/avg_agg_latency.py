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

from copy import copy, deepcopy
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




sys.argv.pop(0)

makeCache = True
#plot the latencies of each individual measure
#print sys.argv

color_comet=sns.xkcd_rgb["medium blue"]
colors=dict()
colors["random"]=sns.xkcd_rgb["medium blue"]
colors["single"]=sns.xkcd_rgb["pale red"]
colors["minimum"]=sns.xkcd_rgb["medium green"]


plt.rcParams.update({'font.size': 20})
plt.figure(figsize=(20,10),dpi=20)

linetype = ['-',':','--']


filenames=sys.argv
#print filenames

suffixes=["_50", "_95", "_99","_99.9", "_99.99", "_soj_50", "_soj_95", "_soj_99","_soj_99.9", "_soj_99.99" ]

avg_results=dict()
for filename in filenames:
    print(filename)
    map_results=dict()
    selections=dict()
    cindex =0
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
        
    avg_results[filename] = map_results

## Second pass
## invert for the sake of averaging
multiRunAverage=dict()

for selection in selections:
    if not selection in multiRunAverage:
        multiRunAverage[selection+"_x"]=[]
        for suffix in suffixes:
            multiRunAverage[selection+suffix]=[]
    for filename in filenames:
        i=0
        for value in avg_results[filename][selection+suffixes[0]]:
            if len(multiRunAverage[selection+suffixes[0]]) <= i:
                multiRunAverage[selection+"_x"].append([])
                for suffix in suffixes:
                    multiRunAverage[selection+suffix].append([])
            multiRunAverage[selection+"_x"][i].append(avg_results[filename][selection+"_x"][i])
            for suffix in suffixes:
                multiRunAverage[selection+suffix][i].append(avg_results[filename][selection+suffix][i])
            i=i+1

#print "AVG RESULTs"
#print avg_results
#print "MULTI RUN AVERAGE"
#print multiRunAverage


#Save the Aggregate Data for another run
pickle.dump(multiRunAverage, open ("Avg_Agg_Latency.db", "wb"))

cindex=0
for selection in selections:
    print ("Final Averages " + selection)
    x=GetAverageArr(multiRunAverage[selection+"_x"])
    y50=GetAverageArr(multiRunAverage[selection+"_50"])
    y50err=GetErrArr(multiRunAverage[selection+"_50"])
    
    y99=GetAverageArr(multiRunAverage[selection+"_99"])
    y99err=GetErrArr(multiRunAverage[selection+"_99"])

    y99_99=GetAverageArr(multiRunAverage[selection+"_99.99"])
    y99_99err=GetAverageArr(multiRunAverage[selection+"_99.99"])

    print (multiRunAverage[selection+"_50"])
    print ("-- Average -- ")
    print (y50)
    plt.plot(x,y50,label=selection+" 50th",color=colors[selection],linestyle=linetype[0])
    plt.errorbar(x,y50,y50err,fmt=' ',color=colors[selection],capsize=5)

    plt.plot(x,y99,label=selection+" 99th",color=colors[selection],linestyle=linetype[2])
    plt.errorbar(x,y99,y99err,fmt=' ',color=colors[selection],capsize=5)

    plt.plot(x,y99_99,label=selection+" 99.99th",color=colors[selection],linestyle=linetype[1])
    plt.errorbar(x,y99_99,y99_99err,fmt=' ',color=colors[selection],capsize=5)
    cindex=cindex+1

plt.rc('axes.formatter', useoffset=False)
plt.legend()
plt.yscale("log")
plt.ylim(top=4000,bottom=30)
plt.grid(True, which="both", ls=":", color='0.20')
plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
plt.ylabel("Response latency (us)", fontweight='bold')
plt.tight_layout()
plt.savefig("Latency.pdf")

plt.clf()

plt.rcParams.update({'font.size': 20})
plt.figure(figsize=(20,10),dpi=20)


network_time=dict()
for selection in selections:
    #copy dimensions to total array
    #sum_redirects[selection]=deepcopy(multiRunAverage[selection+"_NodeRedirections"])
    #sum_total[selection]=deepcopy(multiRunAverage[selection+"_NodeRedirections"])
    network_time[selection+"_50"]=deepcopy(multiRunAverage[selection+"_50"])
    network_time[selection+"_99"]=deepcopy(multiRunAverage[selection+"_99"])
    i = 0
    for measures in multiRunAverage[selection+"_50"]:
        j = 0
        for run in multiRunAverage[selection+"_50"][i]:
            network_sum = multiRunAverage[selection+"_50"][i][j] - multiRunAverage[selection+"_soj_50"][i][j]
            network_time[selection+"_50"][i][j] = network_sum
            network_sum = multiRunAverage[selection+"_99"][i][j] - multiRunAverage[selection+"_soj_99"][i][j]
            network_time[selection+"_99"][i][j] = network_sum
            j=j+1
        i=i+1

for selection in selections:
    x=sorted(GetAverageArr(multiRunAverage[selection+"_x"]))
    avg_50=GetAverageArr(network_time[selection+"_50"])
    err_50=GetErrArr(network_time[selection+"_50"])
    plt.plot(x,avg_50,label=selection,color=colors[selection],linestyle=linetype[0])
    plt.errorbar(x,avg_50,err_50,fmt=' ',color=colors[selection],capsize=5)

    avg_99=GetAverageArr(network_time[selection+"_99"])
    err_99=GetErrArr(network_time[selection+"_99"])
    plt.plot(x,avg_99,label=selection,color=colors[selection],linestyle=linetype[2])
    plt.errorbar(x,avg_99,err_99,fmt=' ',color=colors[selection],capsize=5)

plt.rc('axes.formatter', useoffset=False)
plt.legend()
plt.grid(True, which="both", ls=":", color='0.20')
plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
plt.ylabel("Network Transit Time (us)", fontweight='bold')
plt.tight_layout()
plt.savefig("Network_Latency.pdf")

#network percentage
plt.clf()
plt.rcParams.update({'font.size': 20})
plt.figure(figsize=(20,10),dpi=20)

network_percent=dict()
for selection in selections:

    network_percent[selection+"_50"]=deepcopy(multiRunAverage[selection+"_50"])
    network_percent[selection+"_99"]=deepcopy(multiRunAverage[selection+"_99"])
    i = 0
    for measures in multiRunAverage[selection+"_50"]:
        j = 0
        for run in multiRunAverage[selection+"_50"][i]:
            percent_50 = float(network_time[selection+"_50"][i][j]) / float(multiRunAverage[selection+"_50"][i][j]) * 100.0
            network_percent[selection+"_50"][i][j] = percent_50
            percent_99 = float(network_time[selection+"_99"][i][j]) / float(multiRunAverage[selection+"_99"][i][j]) * 100.0
            network_percent[selection+"_99"][i][j] = percent_99
            j=j+1
        i=i+1


    #TODO plot this with error.
for selection in selections:
    x=sorted(GetAverageArr(multiRunAverage[selection+"_x"]))
    percent_50 = GetAverageArr(network_percent[selection+"_50"])
    err_50=GetErrArr(network_percent[selection+"_50"])
    plt.plot(x,percent_50,label=selection+" 50th",color=colors[selection],linestyle=linetype[0])
    plt.errorbar(x,percent_50,err_50,fmt=' ',color=colors[selection],capsize=5)

    percent_99 = GetAverageArr(network_percent[selection+"_99"])
    err_99=GetErrArr(network_percent[selection+"_99"])
    plt.plot(x,percent_99,label=selection+" 99th",color=colors[selection],linestyle=linetype[2])
    plt.errorbar(x,percent_99,err_99,fmt=' ',color=colors[selection],capsize=5)

plt.rc('axes.formatter', useoffset=False)
plt.legend()
plt.grid(True, which="both", ls=":", color='0.20')
plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
plt.ylabel("Percentage Network Delay", fontweight='bold')
plt.tight_layout()
plt.savefig("Network_Overhead_Percentage.pdf")