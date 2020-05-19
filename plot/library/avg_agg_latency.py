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
print sys.argv

color_comet=sns.xkcd_rgb["medium blue"]
colors=dict()
colors["random"]=sns.xkcd_rgb["medium blue"]
colors["single"]=sns.xkcd_rgb["pale red"]
colors["minimum"]=sns.xkcd_rgb["medium green"]


plt.rcParams.update({'font.size': 20})
plt.figure(figsize=(20,10),dpi=20)

linetype = ['-',':','--']


filenames=sys.argv
print filenames

avg_results=dict()
for filename in filenames:
    print filename
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
                map_results[selection+"_50"]=[]
                map_results[selection+"_95"]=[]
                map_results[selection+"_99"]=[]
                map_results[selection+"_soj_50"]=[]
                map_results[selection+"_soj_95"]=[]
                map_results[selection+"_soj_99"]=[]
            #map_results[selection+"_x"].append(int(row[0]) / 1000) # divide by 1000
            map_results[selection+"_x"].append(int(row[0])) # divide by 1000
            map_results[selection+"_50"].append(float(row[3]))
            map_results[selection+"_95"].append(float(row[4]))
            map_results[selection+"_99"].append(float(row[5]))
            map_results[selection+"_soj_50"].append(float(row[6]))
            map_results[selection+"_soj_95"].append(float(row[7]))
            map_results[selection+"_soj_99"].append(float(row[8]))

        

        #Sort the results
        for selection in selections:
            map_results[selection+"_50"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_50"]))]
            map_results[selection+"_95"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_95"]))]
            map_results[selection+"_99"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_99"]))]
            map_results[selection+"_soj_50"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_soj_50"]))]
            map_results[selection+"_soj_95"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_soj_95"]))]
            map_results[selection+"_soj_99"]=[x for _,x in sorted(zip(map_results[selection+"_x"],map_results[selection+"_soj_99"]))]
            map_results[selection+"_x"]=sorted(map_results[selection+"_x"]) ##Sort X here and no sooner
        
    avg_results[filename] = map_results

## Second pass
## invert for the sake of averating
multiRunAverage=dict()

for selection in selections:
    if not selection in multiRunAverage:
        multiRunAverage[selection+"_x"]=[]
        multiRunAverage[selection+"_50"]=[]
        multiRunAverage[selection+"_95"]=[]
        multiRunAverage[selection+"_99"]=[]
        multiRunAverage[selection+"_soj_50"]=[]
        multiRunAverage[selection+"_soj_95"]=[]
        multiRunAverage[selection+"_soj_99"]=[]
    for filename in filenames:
        i=0
        for value in avg_results[filename][selection+"_50"]:
            if len(multiRunAverage[selection+"_50"]) <= i:
                multiRunAverage[selection+"_x"].append([])
                multiRunAverage[selection+"_50"].append([])
                multiRunAverage[selection+"_95"].append([])
                multiRunAverage[selection+"_99"].append([])
                multiRunAverage[selection+"_soj_50"].append([])
                multiRunAverage[selection+"_soj_95"].append([])
                multiRunAverage[selection+"_soj_99"].append([])
            multiRunAverage[selection+"_x"][i].append(avg_results[filename][selection+"_x"][i])
            multiRunAverage[selection+"_50"][i].append(avg_results[filename][selection+"_50"][i])
            multiRunAverage[selection+"_95"][i].append(avg_results[filename][selection+"_95"][i])
            multiRunAverage[selection+"_99"][i].append(avg_results[filename][selection+"_99"][i])
            multiRunAverage[selection+"_soj_50"][i].append(avg_results[filename][selection+"_soj_50"][i])
            multiRunAverage[selection+"_soj_95"][i].append(avg_results[filename][selection+"_soj_95"][i])
            multiRunAverage[selection+"_soj_99"][i].append(avg_results[filename][selection+"_soj_99"][i])
            i=i+1

#print "AVG RESULTs"
#print avg_results
#print "MULTI RUN AVERAGE"
#print multiRunAverage


#Save the Aggregate Data for another run
pickle.dump(multiRunAverage, open ("Avg_Agg_Latency.db", "wb"))

cindex=0
for selection in selections:
    print "Final Averages " + selection
    x=GetAverageArr(multiRunAverage[selection+"_x"])
    y50=GetAverageArr(multiRunAverage[selection+"_50"])
    y50err=GetErrArr(multiRunAverage[selection+"_50"])
    
    y99=GetAverageArr(multiRunAverage[selection+"_99"])
    y99err=GetErrArr(multiRunAverage[selection+"_99"])
    print multiRunAverage[selection+"_50"]
    print "-- Average -- "
    print y50
    plt.plot(x,y50,label=selection+" 50th",color=colors[selection],linestyle=linetype[0])
    plt.errorbar(x,y50,y50err,fmt=' ',color=colors[selection],capsize=5)

    plt.plot(x,y99,label=selection+" 99th",color=colors[selection],linestyle=linetype[2])
    plt.errorbar(x,y99,y99err,fmt=' ',color=colors[selection],capsize=5)
    cindex=cindex+1

plt.rc('axes.formatter', useoffset=False)
plt.legend()
plt.yscale("log")
plt.ylim(top=4000,bottom=45)
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


for selection in selections:
    #TODO plot this with error.
    x=sorted(GetAverageArr(multiRunAverage[selection+"_x"]))
    avg_latency_50=GetAverageArr(multiRunAverage[selection+"_50"])
    avg_net_50=GetAverageArr(network_time[selection+"_50"])
    percent_50 = [ (a / b) * 100.0 for a, b in zip(avg_net_50,avg_latency_50) ]
    plt.plot(x,percent_50,label=selection,color=colors[selection],linestyle=linetype[0])
    
    avg_latency_99=GetAverageArr(multiRunAverage[selection+"_99"])
    avg_net_99=GetAverageArr(network_time[selection+"_99"])
    percent_99 = [ (a / b) * 100.0 for a, b in zip(avg_net_99,avg_latency_99) ]
    plt.plot(x,percent_99,label=selection,color=colors[selection],linestyle=linetype[2])

plt.rc('axes.formatter', useoffset=False)
plt.legend()
plt.grid(True, which="both", ls=":", color='0.20')
plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
plt.ylabel("Percentage Network Delay", fontweight='bold')
plt.tight_layout()
plt.savefig("Network_Overhead_Percentage.pdf")