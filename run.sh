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
	--TransmissionUniformDistributionMin=$1
	--TransmissionUniformDistributionMax=$2 "
}

function SelectionStrat() {
	select=$1
	echo "--SelectionStrategy=${SelectionStrategy[${select}]} "
}


#take the max and min argument as a parameters
function RunInterval() {
	max=$1
	min=$2

	#Generate transmission arguments for the waf execution
	args=""
	transmission=$(UniformClientTransmission $min $max)
	args="$args 
	$transmission"

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
		#$topdir/waf --run "$topdir/scratch/replication
		currentdir=`pwd`
		pushd $topdir

		./waf --run "scratch/replication
		${cargs}" 2>results.dat

		mv results* $currentdir
		popd

		popd
	done
	
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
		RunInterval $i $i
		popd
	done
}

function PlotStaticIntervalExperiment() {
	dir=`pwd`
	echo "Entering plotting in $dir"
	intervals=(100000 50000 25000 12500 6250 3125 1562)
	#intervals=(50000 25000 12500 6250)

	for i in ${intervals[@]}; do
		dirname="${i}"
		pushd $dirname
		#PlotInterval
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

	PlotStaticIntervalExperiment
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
RunStaticIntervalExperiment

exit
