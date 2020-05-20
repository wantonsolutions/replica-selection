#!/bin/bash



function RunNetLB {
    prefix="EXP"
    rounds=12
    ./run.sh -n="${prefix}_NLB_None" -r=$rounds -f="RunProportionalNone"
    ./run.sh -p
    ./run.sh -n="${prefix}_NLB_Min" -r=$rounds -f="RunProportionalMin"
    ./run.sh -p
    ./run.sh -n="${prefix}_NLB_Core" -r=$rounds -f="RunProportionalCoreOnly"
    ./run.sh -p
    ./run.sh -n="${prefix}_NLB_MDML" -r=$rounds -f="RunProportionalMinDistanceMinLoad"
    ./run.sh -p
    ./run.sh -n="${prefix}_NLB_MDMLC" -r=$rounds -f="RunProportionalMinDistanceMinLoadCore"
    ./run.sh -p
}

function RunNNAverage {
    dirs=()
    count=12
    for i in $(seq 1 $count);do 
        #./run.sh -n="Server_Normal_Client_Normal" -f="RunProportionalLoadNN"
        ./run.sh -n="Normal_Network_CoreOnly_LB" -f="RunProportionalLoadNN"
        echo "completed run $i"
        ./run.sh -p
        dirs+="$(./run.sh --last)/aggregate.dat "
    done
    echo ${dirs[@]}
    ./run.sh --plot_multi --dirs="${dirs[@]}"
}

function PlotKnown {
    #dirs="/home/ssgrant/workspace/ns-allinone-3.29/replica-selection/ns-3.29/Experiments/Server_Normal_Client_Normal_2020-05-07/aggregate.dat /home/ssgrant/workspace/ns-allinone-3.29/replica-selection/ns-3.29/Experiments/Server_Normal_Client_Normal_2020-05-07_1/aggregate.dat /home/ssgrant/workspace/ns-allinone-3.29/replica-selection/ns-3.29/Experiments/Server_Normal_Client_Normal_2020-05-07_2/aggregate.dat"
    prefix="/home/ssgrant/workspace/ns-allinone-3.29/replica-selection/ns-3.29/Experiments/"
    #folder="Normal_Network_LB_2020-05-13"
    folder="Normal_Source_LB_2020-05-13"
    count=11
    file="aggregate.dat"
    for i in $(seq 1 $count);do 
        dirs+="${prefix}${folder}_${i}/${file} "
    done
    dirs"/home/ssgrant/workspace/ns-allinone-3.29/replica-selection/ns-3.29/Experiments/Server_Normal_Client_Normal_2 "

    ./run.sh --plot_multi --dirs="${dirs[@]}"
}

#RunNNAverage
#PlotKnown

RunNetLB