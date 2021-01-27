#!/bin/bash

topdir=`pwd`

declare -A RpcSelectionStrategy
#RpcSelectionStrategy["single"]=0
RpcSelectionStrategy["random"]=1
#RpcSelectionStrategy["minimum"]=2

declare -A NetworkSelectionStrategy
NetworkSelectionStrategy["none"]=0
NetworkSelectionStrategy["minimum"]=1
NetworkSelectionStrategy["coreOnly"]=2
NetworkSelectionStrategy["minDistanceMinLoad"]=3
NetworkSelectionStrategy["coreForcedMinDistanceMinLoad"]=4
NetworkSelectionStrategy["torOnly"]=5
NetworkSelectionStrategy["torQueueDepth"]=6

declare -A ReplicaPlacementAlgorithm
ReplicaPlacementAlgorithm["none"]=0
ReplicaPlacementAlgorithm["random"]=1
ReplicaPlacementAlgorithm["crossCore"]=2
ReplicaPlacementAlgorithm["sameTor"]=3


debug=false

#Init locking functions
startfile="/tmp/startfile"
stopfile="/tmp/stopfile"
max_running="30"

rm $startfile
rm $stopfile
touch $startfile
touch $stopfile

function ConfigArgs() {
	filename=$1
	echo "--Debug=$debug
    --ProbeName=$filename.csv
	--ManifestName=$filename.config"
}

function NumReplicas() {
	replicas="$1"
	echo "--NumReplicas=$1"
}

function ConstantInformationDelay() {
	const=$1
	echo "--InformationDelayFunction=1
    --InformationDelayConst=$const "
}
function UniformClientTransmission() {
	min=$1
	max=$2
	echo "--TransmissionDistributionUniform=true 
	--TransmissionDistributionUniformMin=$min
	--TransmissionDistributionUniformMax=$max "
}

function NormalClientTransmission() {
	mean=$1
	std=$2
	echo "--TransmissionDistributionNormal=true 
	--TransmissionDistributionNormalMean=$mean
	--TransmissionDistributionNormalStd=$std "
}

function ExponentialClientTransmission() {
	lambda=$1
	multiplier=$2
	min=$3
	echo "--TransmissionDistributionExponential=true 
	--TransmissionDistributionExponentialLambda=$lambda
	--TransmissionDistributionExponentialMultiplier=$multiplier
	--TransmissionDistributionExponentialMin=$min "
}

function UniformPacketSizes() {
	min=$1
	max=$2
	echo "--PacketSizeDistributionUniform=true 
	--PacketSizeDistributionUniformMin=$min
	--PacketSizeDistributionUniformMax=$max "
}

function NormalPacketSizes() {
	mean=$1
	std=$2
	echo "--PacketSizeDistributionNormal=true 
	--PacketSizeDistributionNormalMean=$mean
	--PacketSizeDistributionNormalStd=$std "
}

function ExponentialPacketSize() {
	lambda=$1
	multiplier=$2
	min=$3
	echo "--PacketSizeDistributionExponential=true 
	--PacketSizeDistributionExponentialLambda=$lambda
	--PacketSizeDistributionExponentialMultiplier=$multiplier
	--PacketSizeDistributionExponentialMin=$min "
}

function UniformServerLoad() {
	min=$1
	max=$2
	echo "--ServerLoadDistributionUniform=true 
	--ServerLoadDistributionUniformMin=$min
	--ServerLoadDistributionUniformMax=$max "
}

function NormalServerLoad() {
	mean=$1
	std=$2
	echo "--ServerLoadDistributionNormal=true 
	--ServerLoadDistributionNormalMean=$mean
	--ServerLoadDistributionNormalStd=$std "
}

function ExponentialServerLoad() {
	lambda=$1
	multiplier=$2
	min=$3
	echo "--ServerLoadDistributionExponential=true 
	--ServerLoadDistributionExponentialLambda=$lambda
	--ServerLoadDistributionExponentialMultiplier=$multiplier
	--ServerLoadDistributionExponentialMin=$min "
}

function QueueDelta() {
	local delta="$1"
	echo "--QueueDelta=${delta} "
}

function InformationSpread() {
	local spread="$1"
	echo "--InformationSpreadInterval=${spread} "
}

function RpcSelectionStrat() {
	select=$1
	echo "--RpcSelectionStrategy=${RpcSelectionStrategy[${select}]} "
}

function ReplicaPlacementStrat() {
	select=$1
	echo "--ReplicaPlacementAlgorithm=${ReplicaPlacementAlgorithm[${select}]}"
}

function NetworkSelectionStrat() {
	select=$1
	echo "--NetworkSelectionStrategy=${NetworkSelectionStrategy[${select}]} "
}

function WorkingDirectory() {
	dir=$1
	echo "--WorkingDirectory=${dir}/ "
}

function RunRpcSelectionStrategies() {
	local args="$@"
	#run a test for each of the replica selection strategies
	for selection in "${!RpcSelectionStrategy[@]}"; do

		#Create Seperate sub directories for each execution
		echo $selection
		mkdir $selection
		pushd $selection

		#networkArgs=$(NetworkSelectionStrat minimum)

		currentdir=`pwd`
		local dirArgs=$(WorkingDirectory $currentdir)
		args="${args}
		$dirArgs"


		local selectionArgs=$(RpcSelectionStrat $selection)
		args="${args} 
		$selectionArgs "

		local configArgs=$(ConfigArgs results)
		args="${args}
		$configArgs "

		echo ${args}
		pushd $topdir

		./waf --run "scratch/replication
		${args}" 2>${currentdir}/results.dat
		sleep 1
		##tell the controller that its done running
		echo ">>>>" >> $stopfile

		#mv results* $currentdir
		popd

		popd

	done
}

function PlotInterval() {

	#Plot each of the executions in this interval
	plotScript="$topdir/plot/library/raw_latency.py"
	plotArgs=""

	for selection in "${!RpcSelectionStrategy[@]}"; do
		plotArgs="${plotArgs} $selection/results.dat"
	done

	python ${plotScript} ${plotArgs} &

	plotScript="$topdir/plot/library/switch_redirect.py"
	plotArgs="$selection/RouterSummary.dat"

	python ${plotScript} ${plotArgs} &



}


function RunWithArgs {
	args="$@"

	processing="10000"
	shift="5000"
	#have to convert this manually
	lambda="0.005"
	lambdaINV="200"
	local loadArgs=$(ExponentialServerLoad $lambda $processing $shift) #Exponential Distribution

	let "expmean = ($shift + ($processing * $lambdaINV))"
	echo "Expmean = $expmean"
	local proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)


	for p in ${proportion[@]}; do
		#let "mean = (($shift + ($processing * $lambdaINV)) * $p) / 100"
		let "mean = (($shift + ($processing * $lambdaINV)) * 100) / $p"
		echo "Normal Mean = $mean"
		let "std = (mean * 100) / 65"
		#let "std = ${mean} / 10"

		#client mean = Shift + (processing / lambda)
		local transmissionArgs=$(NormalClientTransmission $mean $std)
		#local transmissionArgs=$(ExponentialClientTransmission $lambda $mean 0.0)

		dirname="${p}"
		mkdir $dirname
		pushd $dirname
		RunRpcSelectionStrategies "${args} ${loadArgs} ${transmissionArgs}" &
		sleep 1
		popd

		while true; do
        #Get Current number of running processes
			start=`wc $startfile | awk '{print $1}'`
			stop=`wc $stopfile | awk '{print $1}'`
			if [[ $stop == "" ]]; then
				stop="0"
			fi
			let 'running=start-stop'
			echo "Running Jobs $running max allowed $max_running (start $start stop $stop)"

			#Wait for jobs to finish or start a new one
			if [[ "$running" -gt "$max_running" ]]; then
				sleep 1
			else
				break
			fi
		done

		#exit
	done
}


function RunDelay {

	if [ -z "$DELAY" ]; then
		echo "cannot run delay experiments if a delay variable is not set use --delay= parameter for this function value in us"
		exit 1
	fi
	
	local delay="$DELAY"
	echo $delay

	local processing="250"
	local shift="5000"

	#have to convert this manually
	local lambda="0.05"
	local lambdaINV="20"

	local loadArgs=$(ExponentialServerLoad $lambda $processing $shift) #Exponential Distribution

	local packetArgs=$(NormalPacketSizes 128 12)
	local delayArgs=$(ConstantInformationDelay $delay)

	local placementArgs=$(ReplicaPlacementStrat crossCore)
	local numReplicaArgs=$(NumReplicas 2)
	#local networkArgs=$(NetworkSelectionStrat none) ##nothing in the network...
	local networkArgs=$(NetworkSelectionStrat coreOnly) ##nothing in the network...
	local proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)
	#local proportion=(85)

	let "expmean = ($shift + ($processing * $lambdaINV))"
	echo "Expmean = $expmean" >> /home/ssgrant/mean.delay

	for p in ${proportion[@]}; do
		let "mean = (($shift + ($processing * $lambdaINV)) * 100) / $p"
		echo "Normal Mean = $mean"
		let "std = (mean * 100) / 65"

		local transmissionArgs=$(NormalClientTransmission $mean $std)

		dirname="${p}"
		mkdir $dirname
		pushd $dirname

		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs} ${args} ${delayArgs} ${placementArgs} ${numReplicaArgs}" &
		sleep 1
		popd

		while true; do
        #Get Current number of running processes
			start=`wc $startfile | awk '{print $1}'`
			stop=`wc $stopfile | awk '{print $1}'`
			if [[ $stop == "" ]]; then
				stop="0"
			fi
			let 'running=start-stop'
			echo "Running Jobs $running max allowed $max_running (start $start stop $stop)"

			#Wait for jobs to finish or start a new one
			if [[ "$running" -gt "$max_running" ]]; then
				sleep 1
			else
				break
			fi
		done

		#exit
	done
}


function RunProportialLoadArgs {
	echo "Running Normal Server Load"

	#processing="10000"
	#processing="5000"
	#processing="1000"
	#processing="500"
	#processing="125"
	#processing="500"
	processing="1000"
	shift="10000"
	#have to convert this manually
	#lambda="0.005"
	#lambdaINV="200"
	lambda="0.05"
	lambdaINV="20"
	#lambda="0.5"
	#lambdaINV="2"

	local args="$@"
	#local loadArgs=$(NormalServerLoad $processing $std) #Normal Distribution
	local loadArgs=$(ExponentialServerLoad $lambda $processing $shift) #Exponential Distribution

	local packetArgs=$(NormalPacketSizes 128 12)
	local delayArgs=$(ConstantInformationDelay 0)


	local placementArgs=$(ReplicaPlacementStrat random)
	local numReplicaArgs=$(NumReplicas 3)

	let "expmean = ($shift + ($processing * $lambdaINV))"
	echo "Expmean = $expmean"
	local proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)
	#local proportion=(5 10 25 50 75 90 100)

	for p in ${proportion[@]}; do
		#let "mean = (($shift + ($processing * $lambdaINV)) * $p) / 100"
		let "mean = (($shift + ($processing * $lambdaINV)) * 100) / $p"
		echo "Normal Mean = $mean"
		let "std = (mean * 100) / 65"
		#let "std = ${mean} / 10"

		#client mean = Shift + (processing / lambda)
		local transmissionArgs=$(NormalClientTransmission $mean $std)
		#local transmissionArgs=$(ExponentialClientTransmission $lambda $mean 0.0)

		dirname="${p}"
		mkdir $dirname
		pushd $dirname
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs} ${args} ${delayArgs} ${placementArgs} ${numReplicaArgs}" &
		sleep 1
		popd

		while true; do
        #Get Current number of running processes
			start=`wc $startfile | awk '{print $1}'`
			stop=`wc $stopfile | awk '{print $1}'`
			if [[ $stop == "" ]]; then
				stop="0"
			fi
			let 'running=start-stop'
			echo "Running Jobs $running max allowed $max_running (start $start stop $stop)"

			#Wait for jobs to finish or start a new one
			if [[ "$running" -gt "$max_running" ]]; then
				sleep 1
			else
				break
			fi
		done

		#exit
	done
}

function RunProportionalInformationSpread {
	spread_interval=$1
	local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local queueArgs=$(QueueDelta 0)
	local informationSpreadArgs=$(InformationSpread ${spread_interval})
	RunProportialLoadArgs "${networkArgs} ${queueArgs} ${informationSpreadArgs}"
}

function RunProportionalInformationSpread_500 {
	RunProportionalInformationSpread "500000"
}

function RunProportionalInformationSpread_200 {
	RunProportionalInformationSpread "200000"
}

function RunProportionalInformationSpread_150 {
	RunProportionalInformationSpread "150000"
}

function RunProportionalInformationSpread_100 {
	RunProportionalInformationSpread "100000"
}

function RunProportionalInformationSpread_50 {
	RunProportionalInformationSpread "50000"
}

function RunProportionalInformationSpread_25 {
	RunProportionalInformationSpread "25000"
}

function RunProportionalInformationSpread_10 {
	RunProportionalInformationSpread "10000"
}


function RunProportionalDeltaQueueDepth0 {
	local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local queueArgs=$(QueueDelta 0)
	RunProportialLoadArgs "${networkArgs} ${queueArgs}"
}

function RunProportionalDeltaQueueDepth1 {
	local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local queueArgs=$(QueueDelta 1)
	RunProportialLoadArgs "${networkArgs} ${queueArgs}"
}

function RunProportionalDeltaQueueDepth2 {
	local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local queueArgs=$(QueueDelta 2)
	RunProportialLoadArgs "${networkArgs} ${queueArgs}"
}

function RunProportionalDeltaQueueDepth3 {
	local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local queueArgs=$(QueueDelta 3)
	RunProportialLoadArgs "${networkArgs} ${queueArgs}"
}

function RunProportionalDeltaQueueDepth4 {
	local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local queueArgs=$(QueueDelta 4)
	RunProportialLoadArgs "${networkArgs} ${queueArgs}"
}

function RunProportionalDeltaQueueDepth5 {
	local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local queueArgs=$(QueueDelta 5)
	RunProportialLoadArgs "${networkArgs} ${queueArgs}"
}

function RunProportionalNone {
	local networkArgs=$(NetworkSelectionStrat none)
	RunProportialLoadArgs "${networkArgs} "
}

function RunProportionalMin {
	local networkArgs=$(NetworkSelectionStrat minimum)
	RunProportialLoadArgs "${networkArgs} "
}

function RunProportionalCoreOnly {
	local networkArgs=$(NetworkSelectionStrat coreOnly)
	RunProportialLoadArgs "${networkArgs} "
}

function RunProportionalMinDistanceMinLoad {
	local networkArgs=$(NetworkSelectionStrat minDistanceMinLoad)
	RunProportialLoadArgs "${networkArgs} "
}

function RunProportionalMinDistanceMinLoadCore {
	local networkArgs=$(NetworkSelectionStrat coreForcedMinDistanceMinLoad)
	RunProportialLoadArgs "${networkArgs} "
}

function RunProportionalTorOnly {
	local networkArgs=$(NetworkSelectionStrat torOnly)
	RunProportialLoadArgs "${networkArgs} "
}

function RunProportionalTorQueueDepth {
	local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local queueArgs=$(QueueDelta 2)
	RunProportialLoadArgs "${networkArgs} "
}

function RunReplicaArgs {
	echo "Running Normal Server Load"

	processing="10000"
	shift="5000"
	#have to convert this manually
	lambda="0.005"
	lambdaINV="200"

	local args="$@"
	#local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local networkArgs=$(NetworkSelectionStrat coreForcedMinDistanceMinLoad)
	#local loadArgs=$(NormalServerLoad $processing $std) #Normal Distribution
	local loadArgs=$(ExponentialServerLoad $lambda $processing $shift) #Exponential Distribution

	local packetArgs=$(NormalPacketSizes 128 12)
	local delayArgs=$(ConstantInformationDelay 0)


	local placementArgs=$(ReplicaPlacementStrat crossCore)
	#local placementArgs=$(ReplicaPlacementStrat random)
	#local placementArgs=$(ReplicaPlacementStrat sameTor)

	let "expmean = ($shift + ($processing * $lambdaINV))"
	echo "Expmean = $expmean"
	local proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)

	for p in ${proportion[@]}; do
		#let "mean = (($shift + ($processing * $lambdaINV)) * $p) / 100"
		let "mean = (($shift + ($processing * $lambdaINV)) * 100) / $p"
		echo "Normal Mean = $mean"
		let "std = (mean * 100) / 65"
		#let "std = ${mean} / 10"

		#client mean = Shift + (processing / lambda)
		local transmissionArgs=$(NormalClientTransmission $mean $std)
		#local transmissionArgs=$(ExponentialClientTransmission $lambda $mean 0.0)

		dirname="${p}"
		mkdir $dirname
		pushd $dirname
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs} ${args} ${delayArgs} ${placementArgs} ${numReplicaArgs} ${networkArgs}" &
		sleep 1
		popd

		while true; do
        #Get Current number of running processes
			start=`wc $startfile | awk '{print $1}'`
			stop=`wc $stopfile | awk '{print $1}'`
			if [[ $stop == "" ]]; then
				stop="0"
			fi
			let 'running=start-stop'
			echo "Running Jobs $running max allowed $max_running (start $start stop $stop)"

			#Wait for jobs to finish or start a new one
			if [[ "$running" -gt "$max_running" ]]; then
				sleep 1
			else
				break
			fi
		done

		#exit
	done
}

function RunReplicas1 {
	local numReplicaArgs=$(NumReplicas 1)
	RunReplicaArgs "${replicaArgs} "
}

function RunReplicas2 {
	local numReplicaArgs=$(NumReplicas 2)
	RunReplicaArgs "${replicaArgs} "
}

function RunReplicas3 {
	local numReplicaArgs=$(NumReplicas 3)
	RunReplicaArgs "${replicaArgs} "
}

function RunReplicas4 {
	local numReplicaArgs=$(NumReplicas 4)
	RunReplicaArgs "${replicaArgs} "
}

function RunReplicas5 {
	local numReplicaArgs=$(NumReplicas 5)
	RunReplicaArgs "${replicaArgs} "
}


function RunPlacementArgs {
	echo "Running Placement Strategies"

	local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local packetArgs=$(NormalPacketSizes 128 12)
	local delayArgs=$(ConstantInformationDelay 0)
	local numReplicaArgs=$(NumReplicas 3)
	local placementArgs="$@"
	args="${networkArgs} ${packetArgs} ${delayArgs} ${numReplicaArgs} ${placementArgs}"
	RunWithArgs ${args}
}

function RunPlacementNone {
	RunPlacementArgs $(ReplicaPlacementStrat none)
}

function RunPlacementRandom {
	RunPlacementArgs $(ReplicaPlacementStrat random)
}

function RunPlacementCrossCore {
	RunPlacementArgs $(ReplicaPlacementStrat crossCore)
}
function RunPlacementSameTor {
	RunPlacementArgs $(ReplicaPlacementStrat sameTor)
}


function RunProcessingSkew {
	echo "Running Normal Server Load"

	processing="$@"
	shift="5000"
	#have to convert this manually
	lambda="0.05"
	lambdaINV="20"

	#local networkArgs=$(NetworkSelectionStrat torQueueDepth)
	local networkArgs=$(NetworkSelectionStrat none)
	#local loadArgs=$(NormalServerLoad $processing $std) #Normal Distribution
	local loadArgs=$(ExponentialServerLoad $lambda $processing $shift) #Exponential Distribution

	local packetArgs=$(NormalPacketSizes 128 12)
	local delayArgs=$(ConstantInformationDelay 0)

	local placementArgs=$(ReplicaPlacementStrat crossCore)
	local numReplicaArgs=$(NumReplicas 2)

	let "expmean = ($shift + ($processing * $lambdaINV))"
	echo "Expmean = $expmean"
	local proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)

	for p in ${proportion[@]}; do
		#let "mean = (($shift + ($processing * $lambdaINV)) * $p) / 100"
		let "mean = (($shift + ($processing * $lambdaINV)) * 100) / $p"
		echo "Normal Mean = $mean"
		let "std = (mean * 100) / 65"
		#let "std = ${mean} / 10"

		#client mean = Shift + (processing / lambda)
		local transmissionArgs=$(NormalClientTransmission $mean $std)
		#local transmissionArgs=$(ExponentialClientTransmission $lambda $mean 0.0)

		dirname="${p}"
		mkdir $dirname
		pushd $dirname
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs} ${args} ${delayArgs} ${placementArgs} ${numReplicaArgs}" &
		sleep 1
		popd

		while true; do
        #Get Current number of running processes
			start=`wc $startfile | awk '{print $1}'`
			stop=`wc $stopfile | awk '{print $1}'`
			if [[ $stop == "" ]]; then
				stop="0"
			fi
			let 'running=start-stop'
			echo "Running Jobs $running max allowed $max_running (start $start stop $stop)"

			#Wait for jobs to finish or start a new one
			if [[ "$running" -gt "$max_running" ]]; then
				sleep 1
			else
				break
			fi
		done

		#exit
	done
}

function RunProcessingSkew250 {
	RunProcessingSkew "250"
}

function RunProcessingSkew500 {
 	RunProcessingSkew "500"
}

function RunProcessingSkew1000 {
 	RunProcessingSkew "1000"
}

function RunProcessingSkew2000 {
 	RunProcessingSkew "2000"
}

function RunProcessingSkew5000 {
 	RunProcessingSkew "5000"
}

function RunProcessingSkew10000 {
 	RunProcessingSkew "10000"
}


function  RunDebug {
	debug=true
	echo "Running Debug"

	processing="10000"
	shift="5000"
	#have to convert this manually
	lambda="0.5"
	lambdaINV="2"

	let "expmean = ($shift + ($processing * $lambdaINV))"
	echo "Expmean = $expmean"

	p="50"

	let "mean = (($shift + ($processing * $lambdaINV)) * 100) / $p"
	echo "Normal Mean = $mean"
	let "std = (mean * 100) / 65"
	#let "std = ${mean} / 10"

	#client mean = Shift + (processing / lambda)
	local transmissionArgs=$(NormalClientTransmission $mean $std)

	#transmissionArgs=$(NormalClientTransmission 500000 50000)
	#transmissionArgs=$(NormalClientTransmission 5000000 500000)
	packetArgs=$(NormalPacketSizes 128 12)
	#local loadArgs=$(ExponentialServerLoad 0.1 50000 1) #Exponential Distribution
	local loadArgs=$(ExponentialServerLoad 0.1 5000 1) #Exponential Distribution
	#loadArgs=$(NormalServerLoad 50000 5000)
	#selectionArgs=$(RpcSelectionStrat single)

	selectionArgs=$(RpcSelectionStrat random)
	#networkSelectionArgs=$(NetworkSelectionStrat coreForcedMinDistanceMinLoad)
	networkSelectionArgs=$(NetworkSelectionStrat torQueueDepth)


	local informationSpreadArgs=$(InformationSpread 5000)


	local placementArgs=$(ReplicaPlacementStrat random)
	local numReplicaArgs=$(NumReplicas 3)

	configArgs=$(ConfigArgs results)
	currentdir=`pwd`
	delayArgs=$(ConstantInformationDelay 0)
	dirArgs=$(WorkingDirectory $currentdir)

	args="${transmissionArgs} ${packetArgs} ${loadArgs} ${selectionArgs} ${networkSelectionArgs} ${configArgs} ${dirArgs} ${delayArgs} ${placementArgs} ${numReplicaArgs} ${queueArgs} ${informationSpreadArgs}"

	pushd $topdir

	#./waf --visualize --command-template="gdb --args %s" --run "scratch/replication"
	#./waf --run "scratch/replication" --command-template="gdb --args %s ${args}"  
	./waf --run "scratch/replication ${args}" 

	mv results* $currentdir
	popd #topdir
	popd #debugdir

}

function AggregateIntervalsExperiment {

	#collect aggregate measures
	files=""
	files=`find | grep results.dat`
	aggScript="$topdir/plot/library/percentile_latency.py"
	python $aggScript $files &

	#collect switch aggregate measures
	files=""
	files=`find | grep RouterSummary.dat`
	aggScript="$topdir/plot/library/combine_switch_measurements.py"
	python $aggScript $files &

}

function PlotIntervalsExperiment {
	dirname=`pwd`
	echo "Entering plotting in $dirname"
	for dir in ./*/     # list directories in the form "/tmp/dirname/"
	do
		dir=${dir%*/}      # remove the trailing "/"
		echo ${dir##*/}    # print everything after the final "/"
		pushd $dir
		PlotInterval
		popd
	done

	plotScript="$topdir/plot/library/agg_latency.py"
	python $plotScript aggregate.dat &


	plotScript="$topdir/plot/library/agg_switch_redirect.py"
	python $plotScript aggregate_switch.dat &

	wait

}

function AggregateIntervalExperimentAverage() {
	for a_dir in ./*/; do
		echo "Aggregating in $a_dir"
		pushd $a_dir
		AggregateIntervalsExperiment
		popd
	done
	wait
}

function PlotIntervalExperimentAverage {

	#files=("Server_Normal_Client_Normal_2020-05-06_1/aggregate.dat" "Server_Normal_Client_Normal_2020-05-06_2/aggregate.dat" "Server_Normal_Client_Normal_2020-05-06_3/aggregate.dat")
	#files=$@
	#add file extension
	latency_files=()
	switch_files=()

	#begin by plogging the iterval experiment for each of the sub directories
	for a_dir in ./*/; do
		echo "Plotting in $a_dir"
		pushd $a_dir
		PlotIntervalsExperiment
		popd
		latency_files+=("${a_dir}aggregate.dat")
		switch_files+=("${a_dir}aggregate_switch.dat")
	done


	plotScript="$topdir/plot/library/avg_agg_latency.py"
	echo "Entering Python Plot"
	python $plotScript ${latency_files[@]}

	plotScript="$topdir/plot/library/avg_agg_switch.py"
	echo "Entering Python Plot"
	python $plotScript ${switch_files[@]}

}



function LastExperiment {
	dir=`ls -td -- ./Experiments/*/ | head -n 1`
	echo `realpath $dir`
}

datetime=`date "+%F_%T"`
CurrentDate=`date "+%F"`

#arguement parsing
for i in "$@"; do 

case $i in
	-n=* | --name=*)
	EXPERIMENT_NAME="${i#*=}"
	;;
	-r=* | --runs=*)
	RUNS="${i#*=}"
	;;
	-d=* | --dir=* )
	DIRECTORY="${i#*=}"
	;;
	--dirs=* )
	DIRECTORIES="${i#*=}"
	;;
	--delay=* )
	DELAY="${i#*=}"
	;;
	-f=* | --function=* )
	FUNCTION="${i#*=}"
	;;
	-p | --plot)
	PLOT="true"
	;;
	--plot_multi)
	PLOT_MULTI=true
	;;
	--last )
	LAST=true
	;;
	--debug)
	DEBUG=true
	echo "running debug"
	;;
	-h | --help)
	echo "no proper help message ready to go just read through the code for now"
	;;
	*)
	echo "$i is an invalid argument - take a look at the code silly billy :^)"
	exit 1
esac

done

if [ ! -z "$LAST" ]; then
	echo $(LastExperiment)
	exit
fi


if [ ! -z $PLOT_MULTI ]; then
	echo "Plotting multiple run average (assuming you know what your doing)"

	cd "./Experiments"
	PlotIntervalExperimentAverage ${DIRECTORIES[@]}
	exit
fi
	

if [ ! -z "$PLOT" ]; then
	if [ ! -z "$DIRECTORY" ]; then
		echo "$DIRECTORY exists"
		#test if the directory exists
		if [ ! -d "./Experiments/$DIRECTORY" ]; then
			echo "$DIRECTORY does not exist, exiting"
			exit 1
		fi
		echo "Plotting Directory $DIRECTORY"
		plotDir="./Experiments/$DIRECTORY"
	else
		echo "Plotting the last experiment again"
		plotDir=$(LastExperiment)
	fi

	echo "PLOTTTTTOOOTING"
	echo $plotDIR
	cd $plotDir
	AggregateIntervalExperimentAverage
	wait
	PlotIntervalExperimentAverage
	#exit after plotting
	exit 0
fi

if [ -z "$EXPERIMENT_NAME" ]; then
	echo "An experiment name must be given; see -n"
	exit 0
fi

echo "Compiling"
./waf -j 40

if [ ! -z "$DEBUG" ]; then
	rm -r ./Experiments/debug
	mkdir ./Experiments/debug
	pushd ./Experiments/debug
	RunDebug
	popd
	exit
fi



#Create and name top level experimental directory
ExperimentDir="./Experiments/${EXPERIMENT_NAME}_${CurrentDate}"
if [ -d ${ExperimentDir} ]; then
	count=`ls -dq ${ExperimentDir}* | wc -l`
	echo "${EXPERIMENT_NAME} already performed today $count times"
	echo "Creating sub directory"
	ExperimentDir="${ExperimentDir}_${count}"
else
	echo "First time running Experiment ${EXPERIMENT_NAME}"

fi

mkdir $ExperimentDir
pushd $ExperimentDir



#check to see if the function exists. This is largely for higher level scripting
if [ ! -z $FUNCTION ]; then
	if [ `type -t $FUNCTION`"" == 'function' ]; then
		echo "$FUNCTION is a valid function input"
	else
		echo "$FUNCTION is not an in scope function: Exiting"
		exit
	fi
else 
	#RunUniformTransmissionExperiment
	#RunNormalTransmissionExperiment
	#RunExponentialTransmissionExperiment
	#RunUniformPacketUniformTransmissionExperiment
	#RunNormalServerLoad
	#RunExponentialServerLoad
	FUNCTION=RunProportionalLoad
	echo "No Function Specified running defualt function $FUNCTION"
fi


#determine how many times to run
if [ -z $RUNS ]; then
	RUNS=1
fi

#Acually execute the functions
for run in $(seq 1 $RUNS); do
	mkdir $run
	pushd $run
	$FUNCTION
	popd
	toilet "Done Waiting Round $run/$RUNS ... waiting a bit before starting a new round"
	sleep 5
done
toilet "SYNCRONIZING"
wait

toilet "!!DONE!!"
#wait for any parallel tasks before exiting

popd
exit
