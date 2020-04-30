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

function UniformPacketSize() {
	min=$1
	max=$2
	echo "--PacketSizeDistributionUniform=true 
	--PacketSizeDistributionUniformMin=$min
	--PacketSizeDistributionUniformMax=$max "
}

function NormalPacketSize() {
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

function PlotInterval() {

	#Plot each of the executions in this interval
	plotScript="$topdir/plot/library/raw_latency.py"
	plotArgs=""

	for selection in "${!SelectionStrategy[@]}"; do
		plotArgs="${plotArgs} $selection/results.dat"
	done

	python ${plotScript} ${plotArgs}
}

function RunUniformTransmissionExperiment() {
	intervals=(100000 50000 25000 12500 6250 3125 1562)

	for i in ${intervals[@]}; do
		echo $i
		let "min = $i - ( $i / 2 )"
		let "max = $i + ( $i / 2 )"
		dirname="${i}"
		mkdir $dirname
		pushd $dirname

		transmissionArgs=$(UniformClientTransmission $min $max)
		RunSelectionStrategies ${transmissionArgs}
		popd
	done
}

function RunNormalTransmissionExperiment {
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

		transmissionArgs=$(NormalClientTransmission $mean $std)
		RunSelectionStrategies ${transmissionArgs}

		popd
	done
}

function RunExponentialTransmissionExperiment {
	echo "Running Exponential transmission Experiment"
	intervals=(100000 50000 25000 12500 6250 3125 1562)
	for i in ${intervals[@]}; do
		lambda="1.0"
		let "multiplier = ${i} * 2"
		let "minimum = 1"

		dirname="${i}"
		mkdir $dirname
		pushd $dirname

		transmissionArgs=$(ExponentialClientTransmission $lambda $multiplier $minimum)
		RunSelectionStrategies ${transmissionArgs}
		popd

	done
}

function RunUniformPacketUniformTransmissionExperiment {
	echo "Running "
	#25us mean uniform transmission
	#transmissionArgs=$(UniformClientTransmission 50000 50000)
	transmissionArgs=$(NormalClientTransmission 50000 5000)
	packetSizes=(64 128 256 512 1024 2048 4096 8192 16384 32768)
	for packet_size in ${packetSizes[@]}; do
		packetArgs=$(UniformPacketSize $packet_size $packet_size)

		dirname="${packet_size}"
		mkdir $dirname
		pushd $dirname
		RunSelectionStrategies "${transmissionArgs} ${packetArgs}"
		popd
	done
}

function RunNormalServerLoad {
	echo "Running Normal Server Load"
	#25us mean uniform transmission
	#transmissionArgs=$(UniformClientTransmission 50000 50000)
	transmissionArgs=$(NormalClientTransmission 50000 5000)
	packetArgs=$(NormalPacketSizes 128 12)
	serverLoad=(1 2 4 8 16 32 64 128 256 512 1024)
	for load in ${serverLoad[@]}; do
		let "std = 1 + ($load / 10)"
		loadArgs=$(NormalServerLoad $load $std)

		dirname="${load}"
		mkdir $dirname
		pushd $dirname
		RunSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs}"
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
	-d=* | --dir=* )
	DIRECTORY="${i#*=}"
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
	if [ ! -z "$DIRECTORY"]; then
		#test if the directory exists
		if [! -d "./Experiments/$DIRECTORY" ]; then
			echo "$DIRECTORY does not exist, exiting"
			exit 1
		fi
		echo "Plotting Directory $DIRECTORY"
		plotDir="./Experiments/$DIRECTORY"
	else
		echo "Plotting the last experiment again"
		#cd ./Experiments 
		plotDir=`ls -td -- ./Experiments/*/ | head -n 1`
	fi
	cd $plotDir
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
#RunUniformTransmissionExperiment
#RunNormalTransmissionExperiment
#RunExponentialTransmissionExperiment
#RunUniformPacketUniformTransmissionExperiment
RunNormalServerLoad

exit
