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

if len(filenames) != 1:
    print "This File only plots a single instance of a re-route counter"
    exit()

filename=filenames[0]

counter=0
data=dict()
names=[]
with open(filename,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        if counter == 0:
            for val in row:
                names.append(str(val))
                data[val]=[]
            counter=counter+1
        else:
            subcounter=0
            for val in row:
                print val
                data[names[subcounter]]=int(val)
                subcounter=subcounter+1

print data

total=data["NodeTotal"]
total=+data["EdgeTotal"]
total+=data["AggTotal"]
total+=data["CoreTotal"]

totalR=data["NodeRedirections"]
totalR=+data["EdgeRedirections"]
totalR+=data["AggRedirections"]
totalR+=data["CoreRedirections"]

nodepercent= float(data["NodeRedirections"]) / float(totalR) * 100
edgepercent= float(data["EdgeRedirections"]) / float(totalR) * 100
aggpercent= float(data["AggRedirections"]) / float(totalR) * 100
corepercent= float(data["CoreRedirections"]) / float(totalR) * 100

rerouteP=float(totalR)/float(total) * 100

values=[nodepercent,edgepercent,aggpercent,corepercent,rerouteP]
xnames=['nodes','edge','agg','core','% rerouted']


barlist=plt.bar(xnames,values,color=colors["single"])
barlist[4].set_color(colors['minimum'])



#plt.rc('axes.formatter', useoffset=False)
plt.ylabel("Percentage of Rerouted Packets", fontweight='bold')
plt.tight_layout()
#plt.tight_layout(rect=(0,0.1,1,1))
plt.savefig("PercentRedirections.pdf")
exit()

