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

def setUniformParams():
    plt.rc('axes.formatter', useoffset=False)
    plt.legend()
    plt.grid(True, which="both", ls=":", color='0.20')
    plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
    plt.ylabel("Request Packet Redirections %", fontweight='bold')
    plt.tight_layout()

sys.argv.pop(0)
print sys.argv

#pretty colors
color_comet=sns.xkcd_rgb["medium blue"]
colors=dict()
colors["random"]=sns.xkcd_rgb["medium blue"]
colors["single"]=sns.xkcd_rgb["pale red"]
colors["minimum"]=sns.xkcd_rgb["medium green"]


plt.rcParams.update({'font.size': 20})
#plt.rcParams.update({'figure.figsize': [10, 5]})
plt.figure(figsize=(20,10),dpi=20)

linetype = ['-',':','--']
linedict=dict()
linedict["_NodeRedirections"]=':'
linedict["_EdgeRedirections"]=':'
linedict["_AggRedirections"]='--'
linedict["_CoreRedirections"]='-.'


filenames=sys.argv
print filenames

avg_results=dict()
for filename in filenames:
    print filename
    map_results=dict()
    selections=dict()
    print "Entering a plotting reginme"
    cindex =0
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
            map_results[selection+"_x"]=sorted(map_results[selection+"_x"])
        
    avg_results[filename] = map_results

## Second pass
## invert for the sake of averating
multiRunAverage=dict()

for selection in selections:
    if not selection in multiRunAverage:
        multiRunAverage[selection+"_x"]=[]# divide by 1000
        multiRunAverage[selection+"_NodeRedirections"]=[]
        multiRunAverage[selection+"_NodeTotal"]=[]
        multiRunAverage[selection+"_EdgeRedirections"]=[]
        multiRunAverage[selection+"_EdgeTotal"]=[]
        multiRunAverage[selection+"_AggRedirections"]=[]
        multiRunAverage[selection+"_AggTotal"]=[]
        multiRunAverage[selection+"_CoreRedirections"]=[]
        multiRunAverage[selection+"_CoreTotal"]=[]
    for filename in filenames:
        i=0
        for value in avg_results[filename][selection+"_NodeRedirections"]:
            if len(multiRunAverage[selection+"_NodeRedirections"]) <= i:
                multiRunAverage[selection+"_x"].append([])
                multiRunAverage[selection+"_NodeRedirections"].append([])
                multiRunAverage[selection+"_NodeTotal"].append([])
                multiRunAverage[selection+"_EdgeRedirections"].append([])
                multiRunAverage[selection+"_EdgeTotal"].append([])
                multiRunAverage[selection+"_AggRedirections"].append([])
                multiRunAverage[selection+"_AggTotal"].append([])
                multiRunAverage[selection+"_CoreRedirections"].append([])
                multiRunAverage[selection+"_CoreTotal"].append([])
            multiRunAverage[selection+"_x"][i].append(avg_results[filename][selection+"_x"][i])
            multiRunAverage[selection+"_NodeRedirections"][i].append(avg_results[filename][selection+"_NodeRedirections"][i])
            multiRunAverage[selection+"_NodeTotal"][i].append(avg_results[filename][selection+"_NodeTotal"][i])
            multiRunAverage[selection+"_EdgeRedirections"][i].append(avg_results[filename][selection+"_EdgeRedirections"][i])
            multiRunAverage[selection+"_EdgeTotal"][i].append(avg_results[filename][selection+"_EdgeTotal"][i])
            multiRunAverage[selection+"_AggRedirections"][i].append(avg_results[filename][selection+"_AggRedirections"][i])
            multiRunAverage[selection+"_AggTotal"][i].append(avg_results[filename][selection+"_AggTotal"][i])
            multiRunAverage[selection+"_CoreRedirections"][i].append(avg_results[filename][selection+"_CoreRedirections"][i])
            multiRunAverage[selection+"_CoreTotal"][i].append(avg_results[filename][selection+"_CoreTotal"][i])
            i=i+1

print "AVG RESULTs"
print avg_results
print "MULTI RUN AVERAGE"
print multiRunAverage

#Save the Aggregate Data for another run
pickle.dump(multiRunAverage, open ("Avg_Agg_Switch.db", "wb"))

#Full
cindex=0
for selection in selections:
    print "Final Averages " + selection
    plotnames=["_NodeRedirections","_EdgeRedirections","_AggRedirections","_CoreRedirections"]
    x=GetAverageArr(multiRunAverage[selection+"_x"])
    for pname in plotnames:
        plt.plot(x,GetAverageArr(multiRunAverage[selection+pname]),label=selection+pname,color=colors[selection],linestyle=linedict[pname])
        plt.errorbar(x,GetAverageArr(multiRunAverage[selection+pname]),GetErrArr(multiRunAverage[selection+pname]),fmt=' ',color=colors[selection],capsize=5)
    cindex=cindex+1

setUniformParams()
plt.savefig("Avg_Agg_Switch_All.pdf")
plt.clf()




plt.rcParams.update({'font.size': 20})
#plt.rcParams.update({'figure.figsize': [10, 5]})
plt.figure(figsize=(20,10),dpi=20)
##TODO This is totally wrong, the percentange for each of the runs needs to be calculated first, then the 

#we need to begin by making a sum array across all experiments 
#sum_redirects=dict()
#sum_total=dict()
percent_redirects=dict()
for selection in selections:
    #copy dimensions to total array
    #sum_redirects[selection]=deepcopy(multiRunAverage[selection+"_NodeRedirections"])
    #sum_total[selection]=deepcopy(multiRunAverage[selection+"_NodeRedirections"])
    percent_redirects[selection]=deepcopy(multiRunAverage[selection+"_NodeRedirections"])
    i = 0
    for measures in multiRunAverage[selection+"_NodeRedirections"]:
        j = 0
        Rsum = 0
        Tsum = 0
        for run in multiRunAverage[selection+"_NodeRedirections"][i]:
            Rsum = Rsum + multiRunAverage[selection+"_NodeRedirections"][i][j]
            Rsum = Rsum + multiRunAverage[selection+"_EdgeRedirections"][i][j]
            Rsum = Rsum + multiRunAverage[selection+"_AggRedirections"][i][j]
            Rsum = Rsum + multiRunAverage[selection+"_CoreRedirections"][i][j]

            Tsum = Tsum + multiRunAverage[selection+"_NodeTotal"][i][j]
            Tsum = Tsum + multiRunAverage[selection+"_EdgeTotal"][i][j]
            Tsum = Tsum + multiRunAverage[selection+"_AggTotal"][i][j]
            Tsum = Tsum + multiRunAverage[selection+"_CoreTotal"][i][j]

            percent_redirects[selection][i][j] = (float(Rsum)/float(Tsum) * 100.0)
            j=j+1
        i=i+1


for selection in selections:
    avg=GetAverageArr(percent_redirects[selection])
    err=GetErrArr(percent_redirects[selection])
    x=sorted(GetAverageArr(multiRunAverage[selection+"_x"]))
    plt.plot(x,avg,label=selection,color=colors[selection],linestyle=linetype[1])
    plt.errorbar(x,avg,err,fmt=' ',color=colors[selection],capsize=5)


'''

for selection in selections:
    summerR=[]
    summerR.append(GetAverageArr(multiRunAverage[selection+"_NodeRedirections"]))
    summerR.append(GetAverageArr(multiRunAverage[selection+"_EdgeRedirections"]))
    summerR.append(GetAverageArr(multiRunAverage[selection+"_AggRedirections"]))
    summerR.append(GetAverageArr(multiRunAverage[selection+"_CoreRedirections"]))
    redirects=[sum(x) for x in zip(*summerR)]
    print(redirects)        

    summerT=[]
    summerT.append(GetAverageArr(multiRunAverage[selection+"_NodeTotal"]))
    summerT.append(GetAverageArr(multiRunAverage[selection+"_EdgeTotal"]))
    summerT.append(GetAverageArr(multiRunAverage[selection+"_AggTotal"]))
    summerT.append(GetAverageArr(multiRunAverage[selection+"_CoreTotal"]))
    totals=[sum(x) for x in zip(*summerT)]
    print(totals)

    diff=[(a/b)*100.0 for a, b in zip(redirects,totals)]

    plt.plot(sorted(GetAverageArr(multiRunAverage[selection+"_x"])),diff,label=selection,color=colors[selection],linestyle=linetype[1], marker=".")
'''

setUniformParams()
plt.savefig("Percent_Switch_Redirects.pdf")
plt.clf()