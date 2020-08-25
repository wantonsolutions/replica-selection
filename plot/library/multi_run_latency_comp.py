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
import math

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
#print sys.argv

color_comet=sns.xkcd_rgb["medium blue"]
colors=dict()
colors["random"]=sns.xkcd_rgb["medium blue"]
colors["single"]=sns.xkcd_rgb["pale red"]
colors["minimum"]=sns.xkcd_rgb["medium green"]


plt.rcParams.update({'font.size': 4})
#plt.rcParams.update({'font.size': 1})
#plt.rcParams.update({'figure.figsize': [10, 5]})
plt.figure(figsize=(80,10),dpi=300)
linetype = ['-',':','--']

#figure(num=None, figsize=(15, 15), dpi=80, facecolor='w', edgecolor='k')

filenames=sys.argv
print (filenames)

if len(filenames) < 2:
    print ("Not enough suppiled files, must have at least two")
    exit()

avg_runs=dict()
for filename in filenames:
    print "opening: " + filename
    avg_run=pickle.load( open(filename, "rb"))
    avg_runs[filename]=avg_run

base = filenames[0]
filenames.pop(0) #remove the base
#treatment = filenames[1]

measurement="minimum_50"

#print(avg_runs)
xaxis=GetAverageArr(avg_runs[base]["minimum_x"])
xaxis=[int(x) for x in xaxis]
min_base_avg=GetAverageArr(avg_runs[base][measurement])
min_base_err=GetErrArr(avg_runs[base][measurement])
min_treatments_diff=[]
min_treatments_diff_err=[]
min_treatments_sum=[]
for filename in filenames:
    min_treatments = GetAverageArr(avg_runs[filename][measurement])
    min_treatments_err = GetErrArr(avg_runs[filename][measurement])
    #min_treatments_run=[ ((a-b)/a)*100.0 for a, b in zip(min_base_avg,min_treatments) if a != 0]
    min_treatments_run=[]
    for a, b in zip(min_base_avg,min_treatments):
        print ("A: ",a)
        print ("B: ",b)
        if a != 0 and b != 0:
            min_treatments_run.append(((a-b)/a)*100.0)
            #min_treatments_run.append(b)
        else:
            print "unable to append a, dead run"
            min_treatments_run.append(0)
    
    print ("Min Treatments: ",min_treatments_run)
    min_treatments_run = min_treatments_run[: len(min_treatments_run) ]

    min_treatments_diff.append(min_treatments_run)

    #calculate error
    #a - b error
    p_err = [math.sqrt((((math.sqrt((a_err**2) + (b_err**2)))/(a-b)) ** 2) + ((a_err / a ) ** 2)) for a, a_err, b, b_err in zip(min_base_avg,min_base_err,min_treatments,min_treatments_err)]
    min_treatments_diff_err.append(p_err)
    
    #add to sum
    min_treatments_sum.append(sum(min_treatments_run)/float(len(min_treatments_run)))






#diff=[((a-b)/a)*100.0 for a, b in zip(min_base_avg,min_treatment_avg)]
#print diff

#plt.bar(xaxis,diff,width=4.5,color=colors["minimum"])
fig, ax = plt.subplots()
#im = ax.imshow(min_treatments_diff_err)
im = ax.imshow(min_treatments_diff)


# We want to show all ticks...
print len(min_treatments_run)
ax.set_xticks(np.arange(len(xaxis)))
ax.set_yticks(np.arange(len(filenames)))
# ... and label them with the respective list entries
ax.set_xticklabels(xaxis)
ax.set_yticklabels(filenames)

# Rotate the tick labels and set their alignment.
plt.setp(ax.get_xticklabels(), rotation=45, ha="right",
         rotation_mode="anchor")

# Loop over data dimensions and create text annotations.
for i in range(len(filenames)):
    for j in range(len(min_treatments_run)):
        #text = ax.text(j, i, str(int(min_treatments_diff[i][j]))+" e "+str(float(min_treatments_diff_err[i][j])),
        text = ax.text(j, i, str(int(min_treatments_diff[i][j])),
                       ha="center", va="center", color="w")
#ax = plt.gca()
#ax.ticklabel_format(useOffset=False)

#plt.xscale("log")
#plt.rc('axes.formatter', useoffset=False)
#lgd = plt.legend(ncol=3,loc="lower center",bbox_to_anchor=(0.50,-0.20))
#plt.legend()
#plt.xlabel("Request Load (percentage of maximum)", fontweight='bold')
#plt.xlabel("Server Delay Mean", fontweight='bold')
plt.tight_layout()
#plt.tight_layout(rect=(0,0.1,1,1))
plt.savefig("Heatmap.pdf")

plt.clf()

plt.title("Normalized Percentage Improvement")
plt.bar(filenames,min_treatments_sum)
plt.xticks(rotation=45)
plt.tight_layout()
plt.savefig("Sum_Compairison.pdf")
exit()

