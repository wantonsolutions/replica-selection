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

#figure(num=None, figsize=(15, 15), dpi=80, facecolor='w', edgecolor='k')

filenames=sys.argv
print filenames

if len(filenames) < 2:
    print "Not enough suppiled files, must have at least two"
    exit()

avg_runs=dict()
for filename in filenames:
    print "opening: " + filename
    avg_run=pickle.load( open(filename, "rb"))
    avg_runs[filename]=avg_run

base = filenames[0]
treatment = filenames[1]


xaxis=GetAverageArr(avg_runs[base]["minimum_x"])
min_base_avg=GetAverageArr(avg_runs[base]["minimum_50"])
min_treatment_avg=GetAverageArr(avg_runs[treatment]["minimum_50"])


diff=[((a-b)/a)*100.0 for a, b in zip(min_base_avg,min_treatment_avg)]
print diff

plt.bar(xaxis,diff,width=4.5,color=colors["minimum"])




#ax = plt.gca()
#ax.ticklabel_format(useOffset=False)

#plt.xscale("log")
plt.rc('axes.formatter', useoffset=False)
#lgd = plt.legend(ncol=3,loc="lower center",bbox_to_anchor=(0.50,-0.20))
plt.legend()
plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
#plt.xlabel("Server Delay Mean", fontweight='bold')
plt.ylabel("Percentage Improvement", fontweight='bold')
plt.tight_layout()
#plt.tight_layout(rect=(0,0.1,1,1))
plt.savefig("Min_Source_vs_Network.pdf")
exit()

