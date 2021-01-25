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



print ("Entering a new delta queue plotting script")


sys.argv.pop(0)

#plot the latencies of each individual measure
#print sys.argv

color_comet=sns.xkcd_rgb["medium blue"]
colors=dict()
colors["random"]=sns.xkcd_rgb["medium blue"]
colors["single"]=sns.xkcd_rgb["pale red"]
colors["minimum"]=sns.xkcd_rgb["medium green"]

client_strategy="random"


plt.rcParams.update({'font.size': 18})
#plt.rcParams.update({'font.size': 1})
#plt.rcParams.update({'figure.figsize': [10, 5]})
plt.figure(figsize=(10,5),dpi=20)
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

all_files=filenames



base = filenames[0]
zero_average = filenames[1]
#filenames.pop(0) #remove the base
# so here we know that the fist file is always going to be random
print(avg_runs)
#print avg_runs[base]
xaxis=GetAverageArr(avg_runs[base][client_strategy+"_x"])
xaxis=[int(x) for x in xaxis]

#measurements = [ 'random_50' , 'random_95']
measurements = [ 'random_99.9', 'random_99', 'random_95', 'random_50']
legend_text = ['99.9th', '99th', '95th', '50th']
axis = ["off", "0", "1", "2", "3", "4", "5"]
#measurements = [ 'random_95']

random_average =GetAverageArr(avg_runs[base][measurements[0]])

percent = 70

load_percent_index= (len(random_average) * percent) / 100 
print ("load percent index for " + str(percent) + " = " + str(load_percent_index))


index = 0
for percentile_measurement in measurements:


    fifty = []
    for filename in filenames:
        avg = GetAverageArr(avg_runs[filename][percentile_measurement])
        value = avg[load_percent_index]
        fifty.append(value)


    #value_0 = random_average[load_percent_index]
    #value_1 = queue_zero[load_percent_index]

    #average_50 = [value_0, value_1]
    average_50_bar = plt.bar(axis,fifty,0.55,label=legend_text[index])
    index = index + 1

plt.tight_layout()
plt.legend(ncol=4)
plt.title("")
plt.ylabel("Latency (us)")
plt.xlabel("Delta Value")
plt.show()
plt.savefig("queue_depth.pdf")
