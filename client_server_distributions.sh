#!/bin/bash

prefix="Delay2"
date="2020-06-02"
#dirs=("None" "Min" "Core" "MDML" "MDMLC")
dirs=("0" "3500" "7000" "10500" "14000" "17500" "21000" "24500" "28000")

function RunNetLB {
    rounds=3
    ./run.sh -n="${prefix}_None" -r=$rounds -f="RunProportionalNone"
    ./run.sh -n="${prefix}_Min" -r=$rounds -f="RunProportionalMin"
    #./run.sh -n="${prefix}_Core" -r=$rounds -f="RunProportionalCoreOnly"
    #./run.sh -n="${prefix}_MDML" -r=$rounds -f="RunProportionalMinDistanceMinLoad"
    #./run.sh -n="${prefix}_MDMLC" -r=$rounds -f="RunProportionalMinDistanceMinLoadCore"
}

function RunDelay {
    rounds=3
    ./run.sh -n="${prefix}_0" -r=$rounds -f="Delay0"
    ./run.sh -n="${prefix}_3500" -r=$rounds -f="Delay3500"
    ./run.sh -n="${prefix}_7000" -r=$rounds -f="Delay7000"
    ./run.sh -n="${prefix}_10500" -r=$rounds -f="Delay10500"
    ./run.sh -n="${prefix}_14000" -r=$rounds -f="Delay14000"
    ./run.sh -n="${prefix}_17500" -r=$rounds -f="Delay17500"
    ./run.sh -n="${prefix}_21000" -r=$rounds -f="Delay21000"
    ./run.sh -n="${prefix}_24500" -r=$rounds -f="Delay24500"
    ./run.sh -n="${prefix}_28000" -r=$rounds -f="Delay28000"
}


function PlotKnown {

    for dir in ${dirs[@]}; do
        ./run.sh -p -d="${prefix}_${dir}_${date}"
    done

}

function MoveKnown {
    pushd Experiments

    mkdir ${prefix}
    for dir in ${dirs[@]}; do
        echo $dir
        cp ${prefix}_${dir}_${date}/Avg_Agg_Latency.db ${prefix}/${dir}.db
        cp ${prefix}_${dir}_${date}/Avg_Agg_Switch.db ${prefix}/${dir}_switch.db
    done
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


#RunNetLB
#RunDelay
PlotKnown
MoveKnown
