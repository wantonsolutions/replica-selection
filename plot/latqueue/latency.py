import numpy as np
from pylab import *
from matplotlib.pyplot import figure
from matplotlib.font_manager import FontProperties
import matplotlib.pyplot as plt
import matplotlib.colors as colors
import csv
import sys
import pickle

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
filedict = dict()
if makeCache:    
    suffixes = ["dat","csv"]
    filenames=sys.argv
    filedict = organizeFilenames(filenames, suffixes)
    fullDB = populateDataBases(filedict)
    pickle.dump (fullDB, open("./dbcache.p","w+"))


print "Graphing Queues"

devFields = ['TxBytes',
            'TxPackets',
            'TxDropBytes',
            'TxDropPackets',
            'RxBytes',
            'RxPackets'
            ]

queueFields = ['EnqueueBytes',
            'EnqueuePackets',
            'DropBytes',
            'DropPackets',
            'DequeueBytes',
            'DequeuePackets',
            'OccupancyBytes',
            'OccupancyPackets',
            'MinOccupancyBytes',
            'MinOccupancyPackets',
            'MaxOccupancyBytes',
            'MaxOccupancyPackets',
            ]

appFields = ['Latency',
             'Time',
             'Sent',
             'Received',
             'Request-Index',
             'D-Level',
             'Id',
             ]

color=iter(cm.rainbow(np.linspace(0,1,100)))
c=next(color)
plt.rcParams.update({'font.size': 20})

#Plot Dev Individual
'''
for name in filedict:
            #plotDev(filedict[name][suf],plt,'TxDropPackets','r')
            fig, ax1 = plt.subplots()
            #fig(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')

            ax2 = ax1.twinx()
            ax2 = plotDev(filedict[name]['csv'],ax2,'TxPackets','r')
            ax2.set_ylabel('Enqueued Packets')

            ax1.set_ylabel('Latency (ns)')
            plt.xlabel("Time (S)")
            
            lat = filedict[name]['dat']['Latency']
            time = filedict[name]['dat']['Time']
            ax1.plot(time,lat,'g-',color = 'b',label=name + "--latency")


           

            plt.legend()
            plt.xlabel("Time (Seconds)")
            plt.title(name)
            plt.show()
            plt.clf()
            #figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')
'''

#Plot Dev All

tmpcolor = ['b','r','c','k', 'm', 'g']

plt.xlabel("Time (S)")
plt.title("D-Redundancy v.s UDP-Echo latency")
plt.ylabel('Latency 100s of seconds')
#fig, ax1 = plt.subplots()
cindex =0
#figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')

for name in filedict:
            sname = name.split("/")
            subname = sname[len(sname)-1]
            leftname = subname.split("_")
            finalname = leftname[0]
            
            lat = filedict[name]['dat']['Latency']
            time = filedict[name]['dat']['Time']
            sent = filedict[name]['dat']['Sent']
            rec = filedict[name]['dat']['Received']
            nid = filedict[name]['dat']['Id']


            

            sentAgg = calculateTotalSentRequests(time,sent,rec,nid)
            outstandingAgg = calculateFailedRequests(time,sent,rec,nid)

            #plt.plot(time,sentAgg,'g--',color=tmpcolor[cindex],label=finalname + " Sent-Agg")
            #cindex = cindex + 1
            #plt.plot(time,outstandingAgg,'g--',color=tmpcolor[cindex],label=finalname + " Outstanding-Agg")
            #cindex = cindex + 1
            plt.plot(time,lat,'g--',color=tmpcolor[cindex],label=finalname + " latency")
            cindex = cindex + 1
            c=next(color)
plt.legend()

#plt.show()


#rightaxisplot1='TxDropPackets'
txp='TxPackets'
rxp='RxPackets'
dpt='TxDropPackets'

#rightaxisplot3='EnqueuePackets'
eqp='EnqueuePackets'
dp='DropPackets'


#plt.title("D-Redundancy To UDP Convergance - Packet Loss and Network Fill")
plt.title('Singe Packet Trace')
plt.ylabel('Enqueued Packets (4096 Byte)')
plt.xlabel("Time us")


#ax2 = ax1.twinx()
#ax2.set_ylabel(rightaxisplot1)g,
cindex = 0
for name in filedict:
            #s = sumDevFeild(filedict[name]['csv'], 'TxPackets')
            sname = name.split("/")
            subname = sname[len(sname)-1]
            leftname = subname.split("_")
            finalname = leftname[0]

            #Tx Packets 
            #s = sumDevFeild(filedict[name]['csv'], txp)
            #plt.plot(filedict[name]['csv']['Time'], s, 'g-', color=tmpcolor[cindex],label=txp + " " + finalname)
            #c=next(color)

            #s = avgDevFeild(filedict[name]['csv'], rxp)
            #plt.plot(filedict[name]['csv']['Time'], s, 'g-', color=c,label=rxp + " " + finalname)
            #c=next(color)

            #s = sumDevFeild(filedict[name]['csv'], dpt)
            #plt.plot(filedict[name]['csv']['Time'], s, 'g-*', color=tmpcolor[cindex],label=dpt + " " + finalname)
            #cindex = cindex + 1

            #Enqueue packets
            s = sumQueueFeild(filedict[name]['csv'], eqp)
            plt.plot(filedict[name]['csv']['Time'], s, 'g--', color=tmpcolor[cindex],label=eqp + " " + finalname)
            c=next(color)
            cindex = cindex + 1

            #DroppedPackets
            #s = sumQueueFeild(filedict[name]['csv'], dp)
            #plt.plot(filedict[name]['csv']['Time'], s, 'g--', color128.16, 109.33, 114.532, 116.144, 104.128, 104.128, 104.128, 104.128, 104.128, 104.128, 108.534, 118.938, 104.128, 104.128, 104.128, 104.128, 104.128, 10=tmpcolor[cindex],label=dp + " " + finalname)
            #c=next(color)
            #cindex = cindex + 1


            #plotQueueFeild(filedict[name]['csv'],plt,eqp,tmpcolor[cindex],name)

            #s = sumDevFeild(filedict[name]['csv'], rightaxisplot1)
            #ax2.plot(filedict[name]['csv']['Time'], s, 'g-', color=c,label=rightaxisplot1 + " " + finalname)
            #c=next(color)

            #s = sumDevFeild(filedict[name]['csv'], rightaxisplot2)
            #ax2.plot(filedict[name]['csv']['Time'], s, 'g-', color=c,label=rightaxisplot2 + " " + finalname)
            #c=next(color)

fontP = FontProperties()
fontP.set_size('medium')
#plt.legend(loc='upper right', ncol = 4, prop=10)
plt.legend()
#plt.show()
plt.tight_layout()
plt.savefig("queueSum.png")
plt.clf()
plt.rcParams.update({'font.size': 20})

figure(num=None, figsize=(15, 15), dpi=80, facecolor='w', edgecolor='k')
cindex = 0
maxlat = 0
minlat = 999999


i=7
for name in filedict:
            
            sname = name.split("/")
            subname = sname[len(sname)-1]
            leftname = subname.split("_")
            finalname = leftname[0]
            
            lat = filedict[name]['dat']['Latency']
            time = filedict[name]['dat']['Time']
            mslat = NanoToMicroSeconds(lat)
            #ptime, plat = cdf(time,lat)README
            maxlatT = max(mslat)
            minlatT = min(mslat)
            if maxlatT > maxlat:
                maxlat = maxlatT
            if minlatT < minlat:
                minlat = minlatT

            x, y = gen_cdf(mslat,1000)
            plt.plot(x,y,linewidth=i,label=finalname)

            #plt.hist(mslat,1000, normed=1, histtype='step', cumulative=True, label=finalname, linewidth=i)
            
            i=i-2
            #plt.plot(ptime,plat,'g--',color=tmpcolor[cindex],label=finalname)
            cindex = cindex + 1

#Plot max and min veritical lines, this is for the sake of demonstration

#plt.axvline(x=104, label="Theoretical Min = 104us", linestyle='--', color = 'b', linewidth = 1.5)
#plt.axvline(x=minlat, label="Observed  Min = " + str(minlat) +"us", linestyle='--',color = 'r', linewidth = 1.5)
#plt.axvline(x=208, label="Theoretical Max = 208us", linestyle='--', color = 'b', linewidth = 1.5)
#plt.axvline(x=maxlat, label="Observed  Max = " + str(maxlat) + "us",linestyle = '--', color = 'r', linewidth = 1.5)
plt.grid('on')

plt.xlabel("Time us", fontweight='bold')
#plt.ylabel("Percentile")
#plt.xscale("log")

#plt.xlim(right=250)
lgd = plt.legend(ncol=3,loc="lower center",bbox_to_anchor=(0.50,-0.20))
#plt.ylabel("Total Packets (Round Trip)")
#plt.title("Replica Selection Strategy: single vs 2-Random (No-Server-Load)",fontweight='bold')
#plt.title("Replica Selection Strategy: single vs 2-Random (10us Server Load)",fontweight='bold')
#plt.title("Replica Selection Strategy: single, min, random (10us Server Load)",fontweight='bold')
plt.title("Replica Selection Strategy: single, min, random 1Gbps",fontweight='bold')
plt.tight_layout(rect=(0,0.1,1,1))
plt.savefig("Replica.png")

