#!/bin/bash

prefix="EXP_Y11"
date="2020-05-29"
dirs=("None" "Min" "Core" "MDML" "MDMLC")

function RunNetLB {
    rounds=1
    ./run.sh -n="${prefix}_None" -r=$rounds -f="RunProportionalNone"
    ./run.sh -n="${prefix}_Min" -r=$rounds -f="RunProportionalMin"
    ./run.sh -n="${prefix}_Core" -r=$rounds -f="RunProportionalCoreOnly"
    ./run.sh -n="${prefix}_MDML" -r=$rounds -f="RunProportionalMinDistanceMinLoad"
    ./run.sh -n="${prefix}_MDMLC" -r=$rounds -f="RunProportionalMinDistanceMinLoadCore"
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
PlotKnown
MoveKnown
