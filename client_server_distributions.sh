#!/bin/bash

#prefix="Hope_2"
#date="2020-08-11"
date=`date "+%F"`
#date="2020-08-18"
dirs=("None" "Min" "Core" "MDML" "MDMLC" "Tor")

#dirs=("0" "3500" "7000" "10500" "14000" "17500" "21000" "24500" "28000")
#dirs=("0" "3500" ) 

arguments='
 **HELP** \n
-n name of the experiment must be supplied\n
-r number of rounds defaults to 1\n
-p plot (does not require -n to only plot)\n
-h this message'






function RunRandomLoad {
    ./run.sh -n="${prefix}_None" -r=$RUNS -f="RunProportionalNone"
}

function RunNetLB {
    ./run.sh -n="${prefix}_None" -r=$RUNS -f="RunProportionalNone"
    ./run.sh -n="${prefix}_Min" -r=$RUNS -f="RunProportionalMin"
    ./run.sh -n="${prefix}_Core" -r=$RUNS -f="RunProportionalCoreOnly"
    ./run.sh -n="${prefix}_MDML" -r=$RUNS -f="RunProportionalMinDistanceMinLoad"
    ./run.sh -n="${prefix}_MDMLC" -r=$RUNS -f="RunProportionalMinDistanceMinLoadCore"
    ./run.sh -n="${prefix}_Tor" -r=$RUNS -f="RunProportionalTorOnly"
}

function RunDelay {
    local min="0"
    local step="3500"
    local max="28000"
    for i in $(seq $min $step $max); do
        ./run.sh -n="${prefix}_$i" -r=$RUNS -f="RunDelay" --delay="$i"
        echo "Running delay $i"
    done

}


function PlotKnown {
    for dir in ${dirs[@]}; do
        ./run.sh -p -d="${prefix}_${dir}_${date}"
    done
    wait
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

#arguement parsing
for i in "$@"; do 

case $i in
	-n=* | --name=*)
	prefix="${i#*=}"
	;;
	-r=* | --runs=*)
	RUNS="${i#*=}"
	;;
	-p*)
	PLOT="true"
	;;
	-P*)
	JUSTPLOT="true"
	;;
	-h | --help)
	echo  -e $arguments
	;;
	*)
	echo "$i is an invalid argument - take a look at the code silly billy :^)"
	exit 1
esac

done

if [ ! -z "$JUSTPLOT" ]; then
    echo "Just plotting this round"
    PlotKnown
    MoveKnown
    exit 0
fi

#require 
if [ -z "$RUNS" ]; then
	echo "No runs specified DEFAULT=1 see -r"
    RUNS="1"
fi


if [ ! -z "$prefix" ]; then
    #RunDelay
    RunNetLB
fi
#RunRandomLoad

if [ ! -z "$PLOT" ]; then
    echo "Plotting last set of runs"
    PlotKnown
    MoveKnown
fi