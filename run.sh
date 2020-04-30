#!/bin/bash -x

echo "Hello Experiment"

topdir=`pwd`

declare -A SelectionStrategy
SelectionStrategy["single"]=0
SelectionStrategy["random"]=1
SelectionStrategy["minimum"]=2

debug=false

function ConfigArgs() {
	filename=$1
	echo "--Debug=$debug
    --ProbeName=$filename.csv
	--ManifestName=$filename.config"
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

function SelectionStrat() {
	select=$1
	echo "--SelectionStrategy=${SelectionStrategy[${select}]} "
}

function RunSelectionStrategies() {
	args="$@"
	#run a test for each of the replica selection strategies
	for selection in "${!SelectionStrategy[@]}"; do

		#Create Seperate sub directories for each execution
		echo $selection
		mkdir $selection
		pushd $selection

		selectionArgs=$(SelectionStrat $selection)
		nargs="${args} 
		$selectionArgs "

		configArgs=$(ConfigArgs results)
		cargs="${nargs}
		$configArgs "

		echo ${cargs}
		currentdir=`pwd`
		pushd $topdir

		./waf --run "scratch/replication
		${cargs}" 2>results.dat

		mv results* $currentdir
		popd

		popd

	done
}


#take the max and min argument as a parameters
function RunUniformInterval() {
	min=$1
	max=$2

	#Generate transmission arguments for the waf execution
	args=""
	transmission=$(UniformClientTransmission $min $max)
	args="$args 
	$transmission"
	RunSelectionStrategies "$args"
}

#take the max and min argument as a parameters
function RunNormalInterval() {
	mean=$1
	std=$2

	#Generate transmission arguments for the waf execution
	args=""
	transmission=$(NormalClientTransmission $mean $std)
	args="$args 
	$transmission"
	RunSelectionStrategies "$args"
}

#take the max and min argument as a parameters
function RunExponentialInterval() {
	lambda=$1
	multiplier=$2
	min=$3

	#Generate transmission arguments for the waf execution
	args=""
	transmission=$(ExponentialClientTransmission $lambda $multiplier $min)
	args="$args 
	$transmission"
	RunSelectionStrategies "$args"
}

function PlotInterval() {

	#Plot each of the executions in this interval
	plotScript="$topdir/plot/library/raw_latency.py"
	plotArgs=""

	for selection in "${!SelectionStrategy[@]}"; do
		plotArgs="${plotArgs} $selection/results.dat"
	done

	python ${plotScript} ${plotArgs}
}


function RunStaticIntervalExperiment() {
	intervals=(100000 50000 25000 12500 6250 3125 1562)
	#intervals=(50000 25000 12500 6250)

	for i in ${intervals[@]}; do
		echo $i
		dirname="${i}"
		mkdir $dirname
		pushd $dirname
		RunUniformInterval $i $i
		popd
	done
}

function RunUniformIntervalExperiment_percent() {
	intervals=(100000 50000 25000 12500 6250 3125 1562)

	for i in ${intervals[@]}; do
		echo $i
		let "min = $i - ( $i / 2 )"
		let "max = $i + ( $i / 2 )"
		dirname="${i}"
		mkdir $dirname
		pushd $dirname
		RunUniformInterval $min $max
		popd
	done
}

function RunNormalExperiment {
	echo "Running Normal Experiment"
	intervals=(100000 50000 25000 12500 6250 3125 1562)
	for i in ${intervals[@]}; do
		mean=${i}
		#std="1.0"
		#let "std = ( $i / 10 )" #10% std
		let "std = ( $i / 4 )" #25% std
		dirname="${i}"
		mkdir $dirname
		pushd $dirname
		RunNormalInterval $mean $std
		popd
	done
}

function RunExponentialExperiment {
	echo "Running Exponential Experiment"
	intervals=(100000 50000 25000 12500 6250 3125 1562)
	for i in ${intervals[@]}; do
		lambda="1.0"
		let "multiplier = ${i} * 2"
		let "minimum = 1"

		dirname="${i}"
		mkdir $dirname
		pushd $dirname
		RunExponentialInterval $lambda $multiplier $minimum
		popd

	done
}

function PlotIntervalsExperiment() {
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

	pwd
	#collect aggregate measures
	files=""
	files=`find | grep results.dat`
	aggScript="$topdir/plot/library/percentile_latency.py"
	python $aggScript $files > aggregate.dat

	plotScript="$topdir/plot/library/agg_latency.py"
	python $plotScript aggregate.dat
}

datetime=`date "+%F_%T"`
CurrentDate=`date "+%F"`
echo $datetime

#arguement parsing
for i in "$@"; do 

case $i in
	-n=* | --name=*)
	EXPERIMENT_NAME="${i#*=}"
	;;
	-p | --plot)
	PLOT="true"
	;;
	-h | --help)
	echo "No proper help message ready to go just read through the code for now"
	;;
	*)
	echo "$i is an invalid argument - take a look at the code silly billy :^)"
	exit 1
esac

done

if [ ! -z "$PLOT" ]; then
	echo "Plotting the last experiment again"
	cd ./Experiments 
	latest=`ls -td -- */ | head -n 1`
	cd $latest

	PlotIntervalsExperiment
	#exit after plotting
	exit 0
fi

if [ -z "$EXPERIMENT_NAME" ]; then
	echo "An experiment name must be given; see -n"
	exit 0
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

cd $ExperimentDir
#RunStaticIntervalExperiment
#RunUniformIntervalExperiment_percent
#RunNormalExperiment
RunExponentialExperiment

exit
