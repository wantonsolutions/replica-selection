
import numpy as np
from pylab import *
from matplotlib.pyplot import figure
from matplotlib.font_manager import FontProperties
import matplotlib.pyplot as plt
import matplotlib.colors as colors
import csv
import sys
import pickle
from scipy.stats import sem

## invert for the sake of averating

def column(data, colnum):
    col = []
    for row in data:
        col.append(row[colnum])
    return col

def toFloat(data):
    floats = []
    for d in data:
        floats.append(float(d))
    return floats

def toInt(data):
    ints = []
    for d in data:
        ints.append(int(d))
    return ints

def GetAverageArr(multiRun):
    a=[]
    for runs in multiRun:
        empty_runs = 0
        total=0.0
        for j in runs:
            if j == 0:
                print("Zero Index")
                empty_runs = empty_runs + 1
            total = total + j
        average=total / (float(len(runs)) - empty_runs)
        a.append(average)
    return a

def GetErrArr(multiRun):
    a=[]
    for runs in multiRun:
        #todo calculate standard error
        stderr=sem(runs)
        a.append(stderr)

    return a

def checkFileTuples(filedb, suffixes):
    ##TODO compalain if the suffixes for each file prefix don't line up
    ##Implement later the graph will just crash anyways
    return

def organizeFilenames(filenames, suffixes):
    prefixes = dict()
    for f in filenames:
        sep = f.split('.')
        ext = sep.pop()
        pref = '.'.join(sep)
        if not pref in prefixes:
            prefixes[pref] = dict()
        if ext in suffixes: #probably cause a crash for indexing into a list
            prefixes[pref][ext] = f
    checkFileTuples(prefixes,suffixes)
    return prefixes


def populateDataBases(fileDB):
    fullDb = fileDB
    for prefix in fileDB:
        for suffix in fileDB[prefix]:
            if suffix == "csv":
                fullDb[prefix][suffix] = populateQueueDict(prefix + "." + suffix)
            if suffix == "dat":
                #print prefix,suffix
                fullDb[prefix][suffix] = populateAppDict(prefix + "." + suffix)
    return fullDb



def populateQueueDict(filename):
    data = []
    feilds = []
    with open(filename,'r') as csvfile:
        plots = csv.reader(csvfile, delimiter=',')
        line = 0
        for row in plots:
            if line == 0:
                for identifier in row:
                    feilds.append(identifier)
                    #print identifier.split('.')
            else:
                for val in row:
                    data[line-1].append(val)
            line = line + 1
            data.append([])
        data.pop()

    db = dict()

    index = 0
    for f in feilds:
        dels = f.split('.')
        if len(dels) == 1: ## TIME
            db[dels[0]] = toFloat(column(data,0))
        if len(dels) == 3:
            if not (dels[0] in db):           #Nodes
                db[dels[0]] = dict()
            if not (dels[1] in db[dels[0]]):  #Devices
                db[dels[0]][dels[1]] = dict()
            db[dels[0]][dels[1]][dels[2]] = toInt(column(data, index))
        if len(dels) == 4:
            if not (dels[0] in db):                     #Nodes
                db[dels[0]] = dict()
            if not (dels[1] in db[dels[0]]):            #Devices
                db[dels[0]][dels[1]] = dict()
            if not (dels[2] in db[dels[0]][dels[1]]):   #Queues
                db[dels[0]][dels[1]][dels[2]] = dict()
            db[dels[0]][dels[1]][dels[2]][dels[3]] = toInt(column(data, index))
        index = index + 1
    return db

def populateAppDict(filename):
    data = []
    data.append([])
    feilds = []
    with open(filename,'r') as csvfile:
        plots = csv.reader(csvfile, delimiter=',')
        line = 0
        for row in plots:
            for val in row:
                data[line].append(val)
            line = line + 1
            data.append([])
        data.pop()

    db = dict()
    ##EVerything is known parse with more verbosity later
    db["Latency"] = toFloat(column(data, 0))
    db["Time"] = toFloat(column(data,1))
    db["Sent"] = toInt(column(data,2))
    db["Received"] = toInt(column(data,3))
    db["Request-Index"] = toInt(column(data,4))
    db["D-Level"] = toInt(column(data,5))
    db["Id"] = toInt(column(data,6))
    db["Sojourn"] = toInt(column(data,7))
    return db

def sumDevFeild(ddb, name):
    sumlist = []
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            ##print ddb[n][dev]
            index = 0
            if name in ddb[n][dev]:
                if len(sumlist) == 0:
                    #print(name)
                    sumlist = ddb[n][dev][name]
                else:
                    for val in ddb[n][dev][name]:
                        sumlist[index] = sumlist[index] + val
                        index = index + 1
    return sumlist


def avgDevFeild(ddb, name):
    sumf = sumDevFeild(ddb, name)
    total = 0
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            if name in ddb[n][dev]:
                total = total + 1
        break
    #print total
    avg = sumf
    index = 0
    for val in avg:
        avg[index] = float(val) / float(total)
        index += 1

    return avg

def sumQueueFeild(ddb, name):
    sumlist = []
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            for queue in ddb[n][dev]:
                index = 0
                if name in ddb[n][dev][queue]:
                    if len(sumlist) == 0:
                        #print(name)
                        sumlist = ddb[n][dev][queue][name]
                    else:
                        for val in ddb[n][dev][queue][name]:
                            sumlist[index] = sumlist[index] + val
                            index = index + 1
    return sumlist

def avgQueueFeild(ddb, name):
    sumf = sumQueueFeild(ddb, name)
    total = 0
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            for queue in ddb[n][dev]:
                if name in ddb[n][dev][queue]:
                    total = total + 1
        break
    #print total
    avg = sumf
    index = 0
    for val in avg:
        avg[index] = float(val) / float(total)
        index += 1

    return avg

globalcount = 0

def plotQueueFeild(qdb, plt, name, c, fname):

    colorp=iter(cm.rainbow(np.linspace(0,1,40)))
    cp=next(color)
    mlat = SecondsToMicro(qdb['Time'])
    #print(mlat)
    
    tmpcolor = ['b','r','c','k', 'm', 'g']
    cindex=0
    for n in qdb:
        if n == "Time":
            continue
        for dev in qdb[n]:
            for queue in qdb[n][dev]:
                if name in qdb[n][dev][queue]:
                    zeroval = True
                    for v in qdb[n][dev][queue][name]:
                        if v != 0.0:
                            zeroval = False
                    if not zeroval:
                        #plt.plot(mlat,qdb[n][dev][queue][name],'g-', color = tmpcolor[cindex%len(tmpcolor)],label= dev + queue + name)
                        plt.plot(mlat,qdb[n][dev][queue][name],'g-', color=cp,label= dev + queue + name + str(cindex) + fname, linewidth = 2)
                        #print(cp)
                        #print(dev + " " + queue + " " + name)
                        cp=next(colorp)
                        cindex += 1

                        

def plotDev(ddb, plt,name, c):
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            ##print ddb[n][dev]
            if name in ddb[n][dev]:
                plt.plot(ddb['Time'],ddb[n][dev][name],'g:', color=c,label=name)
    return plt

def cdf(time, data):
    high = max(data)
    low = min(data)
    norm = colors.Normalize(low,high)
    ndata = norm(data)
    H, X1 = np.histogram(ndata, bins = 100 )
    dx = X1[1] - X1[0]
    F1 = np.cumsum(H) * dx
    return X1[1:], F1
    #sortedtime = np.sort(time)
    #pdata = 1.0 *np.arange(len(time))/(len(time)-1)
    #return sortedtime, pdata

def gen_cdf(npArray, num_bin):
   array = np.sort(npArray)
   h, edges = np.histogram(array, density=True, bins=num_bin )
   h = np.cumsum(h)/np.cumsum(h).max()
   x = edges.repeat(2)[:-1]
   y = np.zeros_like(x)
   y[1:] = h.repeat(2)
   return x, y


def calculateTotalSentRequests(time,sent,rec,nid):
    sentAgg = dict()
    i = 0
    diff = []
    while i < len(time):
        sentAgg[str(nid[i])] = sent[i]
        s = 0
        for r in sentAgg:
            s += sentAgg[r]
        diff.append(s)
        i += 1
    return diff


def calculateFailedRequests(time,sent,rec,nid):
    outstandingAgg = dict()
    i = 0
    diff = []
    while i < len(time):
        outstandingAgg[str(nid[i])] = sent[i] - rec[i]
        #outstandingAgg[str(nid[i])] = i
        s = 0
        for outstanding in outstandingAgg:
            s += outstandingAgg[outstanding]
        diff.append(s)
        i += 1
    return diff

def NanoToMicroSeconds(lat):
    i = 0
    for val in lat:
        lat[i] = val/1000
        i = i+1
    #print lat
    return lat

def SecondsToMicro(lat):
    i = 0
    for val in lat:
        lat[i] = val*1000*1000
        i = i+1
    #print lat
    return lat