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
#plt.rcParams.update({'figure.figsize': [10, 5]})
plt.figure(figsize=(20,10),dpi=20)

linetype = ['-',':','--']


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
            map_results[selection+"_x"]=sorted(map_results[selection+"_x"])
        
    avg_results[filename] = map_results

## Second pass
## invert for the sake of averating
multiRunAverage=dict()

for selection in selections:
    if not selection in multiRunAverage:
        multiRunAverage[selection+"_50"]=[]
        multiRunAverage[selection+"_95"]=[]
        multiRunAverage[selection+"_99"]=[]
        multiRunAverage[selection+"_x"]=[]
    for filename in filenames:
        i=0
        for value in avg_results[filename][selection+"_50"]:
            if len(multiRunAverage[selection+"_50"]) <= i:
                multiRunAverage[selection+"_50"].append([])
                multiRunAverage[selection+"_95"].append([])
                multiRunAverage[selection+"_99"].append([])
                multiRunAverage[selection+"_x"].append([])
            multiRunAverage[selection+"_50"][i].append(avg_results[filename][selection+"_50"][i])
            multiRunAverage[selection+"_95"][i].append(avg_results[filename][selection+"_95"][i])
            multiRunAverage[selection+"_99"][i].append(avg_results[filename][selection+"_99"][i])
            multiRunAverage[selection+"_x"][i].append(avg_results[filename][selection+"_x"][i])
            i=i+1

print "AVG RESULTs"
print avg_results
print "MULTI RUN AVERAGE"
print multiRunAverage




#for filename in filenames:
#    cindex=0
#    for selection in selections:
#        print avg_results[filename][selection+"_x"]
#        print avg_results[filename][selection+"_50"]
#
#        plt.plot(avg_results[filename][selection+"_x"],avg_results[filename][selection+"_50"],label=selection+"_50"+str(cindex),color=colors[cindex],linestyle=linetype[0], marker="x")
#        #plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_95"],label=selection+"_95",color=colors[cindex],linestyle=linetype[1], marker="x")
#        plt.plot(avg_results[filename][selection+"_x"],avg_results[filename][selection+"_99"],label=selection+"_99"+str(cindex),color=colors[cindex],linestyle=linetype[2], marker="x")
#        cindex=cindex+1

#Save the Aggregate Data for another run
pickle.dump(multiRunAverage, open ("Avg_Agg_Latency.db", "wb"))

cindex=0
for selection in selections:
    print "Final Averages " + selection
    x=GetAverageArr(multiRunAverage[selection+"_x"])
    #print x
    #print(multiRunAverage[selection+"_50"])
    y50=GetAverageArr(multiRunAverage[selection+"_50"])
    y50err=GetErrArr(multiRunAverage[selection+"_50"])
    
    y99=GetAverageArr(multiRunAverage[selection+"_99"])
    y99err=GetErrArr(multiRunAverage[selection+"_99"])
    #print y99
    plt.plot(x,y50,label=selection+" 50th",color=colors[selection],linestyle=linetype[0])
    plt.errorbar(x,y50,y50err,fmt=' ',color=colors[selection],capsize=5)
    #plt.plot(sorted(map_results[selection+"_x"]),map_results[selection+"_95"],label=selection+"_95",color=colors[cindex],linestyle=linetype[1], marker="x")
    plt.plot(x,y99,label=selection+" 99th",color=colors[selection],linestyle=linetype[2])
    plt.errorbar(x,y99,y99err,fmt=' ',color=colors[selection],capsize=5)
    cindex=cindex+1





#ax = plt.gca()
#ax.ticklabel_format(useOffset=False)

#plt.xscale("log")
plt.rc('axes.formatter', useoffset=False)
#lgd = plt.legend(ncol=3,loc="lower center",bbox_to_anchor=(0.50,-0.20))
plt.legend()
#plt.xlim(left=0.9,right=200)
#plt.ylim(top=2500,bottom=-50)
plt.yscale("log")
plt.ylim(top=4000,bottom=45)
#plt.ylim(top=4000,bottom=0)
#plt.grid('on')
plt.grid(True, which="both", ls=":", color='0.20')
#plt.xlabel("Interval between requests (us)", fontweight='bold')
#plt.xlabel("Packet Size (bytes)", fontweight='bold')
plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
#plt.xlabel("Server Delay Mean", fontweight='bold')
plt.ylabel("Response latency (us)", fontweight='bold')
#plt.title("Static request interval response latency",fontweight='bold')
#plt.title("Uniform Requests 25us Variable Packet Sizes",fontweight='bold')
#plt.title("Uniform Clients - Uniform Servers (50us mean)",fontweight='bold')
#plt.title("Normal Clients - Uniform Servers (50us mean)",fontweight='bold')
#plt.title("Normal Clients - Normal Servers (50us mean)",fontweight='bold')
plt.tight_layout()
#plt.tight_layout(rect=(0,0.1,1,1))
plt.savefig("Avg_Agg_Latency.pdf")

