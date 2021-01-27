#!/bin/bash

#prefix="Hope_2"
#date="2020-08-11"
date=`date "+%F"`
#date="2020-08-18"
#dirs=("None" "Min" "Core" "MDML" "MDMLC" "Tor" "TorQ")
#dirs=("None" "MDMLC" "TorQ")
#dirs=("None" "MDMLC")
#dirs=("None" "TorQ")
#dirs=("None" "TorQ_0" "TorQ_1" "TorQ_2" "TorQ_3" "TorQ_4" "TorQ_5")
#dirs=("None" "TorQ_5")
#dirs=("1" "2" "3" "4" "5")

#load spread
dirs=("None" "spread_500" "spread_200" "spread_150" "spread_100" "spread_50" "spread_25" "spread_10")

#dirs=("None" "Random" "CrossCore" "SameTor")

#dirs=("250" "500" "1000" "2000" "5000" "10000")

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
   #./run.sh -n="${prefix}_Min" -r=$RUNS -f="RunProportionalMin"
    #./run.sh -n="${prefix}_Core" -r=$RUNS -f="RunProportionalCoreOnly"
   # ./run.sh -n="${prefix}_MDML" -r=$RUNS -f="RunProportionalMinDistanceMinLoad"
  #  ./run.sh -n="${prefix}_MDMLC" -r=$RUNS -f="RunProportionalMinDistanceMinLoadCore"
  # ./run.sh -n="${prefix}_Tor" -r=$RUNS -f="RunProportionalTorOnly"
    ./run.sh -n="${prefix}_TorQ" -r=$RUNS -f="RunProportionalTorQueueDepth"
}

function RunQueueDepth {
    ./run.sh -n="${prefix}_None" -r="$RUNS" -f="RunProportionalNone"
    ./run.sh -n="${prefix}_TorQ_0" -r="$RUNS" -f="RunProportionalDeltaQueueDepth0"
    ./run.sh -n="${prefix}_TorQ_1" -r="$RUNS" -f="RunProportionalDeltaQueueDepth1"
    ./run.sh -n="${prefix}_TorQ_2" -r="$RUNS" -f="RunProportionalDeltaQueueDepth2"
    ./run.sh -n="${prefix}_TorQ_3" -r="$RUNS" -f="RunProportionalDeltaQueueDepth3"
    ./run.sh -n="${prefix}_TorQ_4" -r="$RUNS" -f="RunProportionalDeltaQueueDepth4"
    ./run.sh -n="${prefix}_TorQ_5" -r="$RUNS" -f="RunProportionalDeltaQueueDepth5"
}

function RunLoadSpread {
    ./run.sh -n="${prefix}_None" -r="$RUNS" -f="RunProportionalNone"
    ./run.sh -n="${prefix}_spread_500" -r="$RUNS" -f="RunProportionalInformationSpread_500"
    ./run.sh -n="${prefix}_spread_200" -r="$RUNS" -f="RunProportionalInformationSpread_200"
    ./run.sh -n="${prefix}_spread_150" -r="$RUNS" -f="RunProportionalInformationSpread_150"
    ./run.sh -n="${prefix}_spread_100" -r="$RUNS" -f="RunProportionalInformationSpread_100"
    ./run.sh -n="${prefix}_spread_50" -r="$RUNS" -f="RunProportionalInformationSpread_50"
    ./run.sh -n="${prefix}_spread_25" -r="$RUNS" -f="RunProportionalInformationSpread_25"
    ./run.sh -n="${prefix}_spread_10" -r="$RUNS" -f="RunProportionalInformationSpread_10"
    #./run.sh -n="${prefix}_spread_5" -r="$RUNS" -f="RunProportionalInformationSpread_5"
    #./run.sh -n="${prefix}_spread_1" -r="$RUNS" -f="RunProportionalInformationSpread_1"
}


function RunReplicas {
    ./run.sh -n="${prefix}_1" -r=$RUNS -f="RunReplicas1"
    ./run.sh -n="${prefix}_2" -r=$RUNS -f="RunReplicas2"
    ./run.sh -n="${prefix}_3" -r=$RUNS -f="RunReplicas3"
    ./run.sh -n="${prefix}_4" -r=$RUNS -f="RunReplicas4"
    ./run.sh -n="${prefix}_5" -r=$RUNS -f="RunReplicas5"
}

function RunPlacement {
    ./run.sh -n="${prefix}_None" -r=$RUNS -f="RunPlacementNone"
    ./run.sh -n="${prefix}_Random" -r=$RUNS -f="RunPlacementRandom"
    ./run.sh -n="${prefix}_CrossCore" -r=$RUNS -f="RunPlacementCrossCore"
    ./run.sh -n="${prefix}_SameTor" -r=$RUNS -f="RunPlacementSameTor"
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

function RunSkew {
    ./run.sh -n="${prefix}_250" -r=$RUNS -f="RunProcessingSkew250"
    ./run.sh -n="${prefix}_500" -r=$RUNS -f="RunProcessingSkew500"
    ./run.sh -n="${prefix}_1000" -r=$RUNS -f="RunProcessingSkew1000"
    ./run.sh -n="${prefix}_2000" -r=$RUNS -f="RunProcessingSkew2000"
    ./run.sh -n="${prefix}_5000" -r=$RUNS -f="RunProcessingSkew5000"
    ./run.sh -n="${prefix}_10000" -r=$RUNS -f="RunProcessingSkew10000"
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
    popd
}

function PlotAgg {
    pushd Experiments/${prefix}
    mkdir switch
    mv *_switch.db switch
    mkdir node
    mv *.db node
    pushd node

    pwd

    args=""
    for dir in ${dirs[@]}; do
        args="$args ${dir}.db"
    done
    python ../../../plot/library/multi_run_latency_comp.py "50" $args
    python ../../../plot/library/multi_run_latency_comp.py "95" $args
    python ../../../plot/library/multi_run_latency_comp.py "99" $args
    python ../../../plot/library/multi_run_latency_comp.py "99.9" $args
    python ../../../plot/library/multi_run_latency_comp.py "99.99" $args
    mv *.pdf ../
    popd 
    popd
}

function PlotDeltaQueue {
    pushd Experiments/${prefix}
    mkdir switch
    mv *_switch.db switch
    mkdir node
    mv *.db node
    pushd node

    pwd

    #arguments are just the directory names
    args=""
    for dir in ${dirs[@]}; do
        args="$args ${dir}.db"
    done
    python ../../../plot/library/delta_queue.py $args
    mv *.pdf ../
    popd 
    popd
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
    PlotAgg
    #PlotDeltaQueue
    exit 0
fi

#require 
if [ -z "$RUNS" ]; then
	echo "No runs specified DEFAULT=1 see -r"
    RUNS="1"
fi


if [ ! -z "$prefix" ]; then
    #RunDelay
    #RunNetLB
    #RunQueueDepth
    #RunSkew
    #RunReplicas
    #RunPlacement
    RunLoadSpread
fi
#RunRandomLoad

if [ ! -z "$PLOT" ]; then
    echo "Plotting last set of runs"
    PlotKnown
    MoveKnown
    PlotAgg
fi