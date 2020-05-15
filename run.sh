#!/bin/bash -x

topdir=`pwd`

declare -A RpcSelectionStrategy
RpcSelectionStrategy["single"]=0
RpcSelectionStrategy["random"]=1
RpcSelectionStrategy["minimum"]=2

declare -A NetworkSelectionStrategy
NetworkSelectionStrategy["none"]=0
NetworkSelectionStrategy["minimum"]=1
NetworkSelectionStrategy["coreOnly"]=2

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

function RpcSelectionStrat() {
	select=$1
	echo "--RpcSelectionStrategy=${RpcSelectionStrategy[${select}]} "
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
	args="$@"
	#run a test for each of the replica selection strategies
	for selection in "${!RpcSelectionStrategy[@]}"; do

		#Create Seperate sub directories for each execution
		echo $selection
		mkdir $selection
		pushd $selection

		#networkArgs=$(NetworkSelectionStrat minimum)

		currentdir=`pwd`
		dirArgs=$(WorkingDirectory $currentdir)
		args="${args}
		$dirArgs"

		networkArgs=$(NetworkSelectionStrat minimum)
		args="${args}
		$networkArgs"

		selectionArgs=$(RpcSelectionStrat $selection)
		args="${args} 
		$selectionArgs "

		configArgs=$(ConfigArgs results)
		args="${args}
		$configArgs "

		echo ${args}
		pushd $topdir

		./waf --run "scratch/replication
		${args}" 2>${currentdir}/results.dat &
		sleep 1

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
		RunRpcSelectionStrategies ${transmissionArgs}
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
		RunRpcSelectionStrategies ${transmissionArgs}

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
		RunRpcSelectionStrategies ${transmissionArgs}
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
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs}"
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
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs}"
		popd
	done
}

function RunExponentialServerLoad {
	echo "Running Normal Server Load"
	#25us mean uniform transmission
	#transmissionArgs=$(UniformClientTransmission 50000 50000)
	transmissionArgs=$(NormalClientTransmission 50000 5000)
	packetArgs=$(NormalPacketSizes 128 12)
	serverLoad=(1 2 4 8 16 32 64 128 256 512)
	for load in ${serverLoad[@]}; do
		lambda="1.0"
		let "multiplier = ${load} * 2"
		let "minimum = 1"
		loadArgs=$(ExponentialServerLoad $lambda $multiplier $minimum)

		dirname="${load}"
		mkdir $dirname
		pushd $dirname
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs}"
		popd
	done
}

function RunProportionalLoad200 {
	echo "Running Normal Server Load"
	#25us mean uniform transmission
	#transmissionArgs=$(UniformClientTransmission 50000 50000)
	loadArgs=$(NormalServerLoad 50000 5000)
	#loadArgs=$(UniformServerLoad 45 55)
	packetArgs=$(NormalPacketSizes 128 12)
	#clientTransmission=(1000 10000 20000 30000 40000 50000 60000 70000 80000 90000 100000)
	#proportion=(20 40 60 80 100 120 140 160 180 200)
	#proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100 105 110 115 120 125 130 135 140 145 150 155 160 165 170 175 180 185 190 195 200)
	proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)
	#proportion=(10 20 30 40 50 60 70 80 90 100)

	#proportion=(50 55 60 65 70 75 80 85 90 95 100)
	#proportion=(50 75 100)

	for p in ${proportion[@]}; do
		let "mean = (50000 * 100) / $p"
		let "std = ${mean} / 10"
		transmissionArgs=$(NormalClientTransmission $mean $std)

		#let "min = $mean - ( $std * 2 )"
		#let "max = $mean + ( $std * 2 )"
		#transmissionArgs=$(UniformClientTransmission $min $max)


		dirname="${p}"
		mkdir $dirname
		pushd $dirname
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs}"
		popd

		#exit
	done
}

function RunProportionalLoadNN {
	echo "Running Normal Server Normal Client"
	loadArgs=$(NormalServerLoad 50000 5000)
	packetArgs=$(NormalPacketSizes 128 12)
	proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)
	#proportion=(50 75 100)

	for p in ${proportion[@]}; do
		let "mean = (50000 * 100) / $p"
		let "std = ${mean} / 10"
		transmissionArgs=$(NormalClientTransmission $mean $std)

		dirname="${p}"
		mkdir $dirname
		pushd $dirname
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs}"
		popd
	done
}

function RunProportionalLoadNU {
	echo "Running Normal Server Uniform Client"
	loadArgs=$(NormalServerLoad 50000 5000)
	packetArgs=$(NormalPacketSizes 128 12)
	proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)

	for p in ${proportion[@]}; do
		let "mean = (50000 * 100) / $p"
		let "std = ${mean} / 10"
		let "min = $mean - ( $std * 2 )"
		let "max = $mean + ( $std * 2 )"
		transmissionArgs=$(UniformClientTransmission $min $max)

		dirname="${p}"
		mkdir $dirname
		pushd $dirname
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs}"
		popd
	done
}

function RunProportionalLoadUN {
	echo "Running Uniform Server Normal Client"
	loadArgs=$(UniformServerLoad 45000 55000)
	packetArgs=$(NormalPacketSizes 128 12)
	proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)

	for p in ${proportion[@]}; do
		let "mean = (50000 * 100) / $p"
		let "std = ${mean} / 10"
		transmissionArgs=$(NormalClientTransmission $mean $std)

		dirname="${p}"
		mkdir $dirname
		pushd $dirname
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs}"
		popd
	done
}

function RunProportionalLoadUU {
	echo "Running Uniform Server Normal Client"
	loadArgs=$(UniformServerLoad 45000 55000)
	packetArgs=$(NormalPacketSizes 128 12)
	proportion=(5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100)

	for p in ${proportion[@]}; do
		let "mean = (50000 * 100) / $p"
		let "std = ${mean} / 10"
		let "min = $mean - ( $std * 2 )"
		let "max = $mean + ( $std * 2 )"
		transmissionArgs=$(UniformClientTransmission $min $max)


		dirname="${p}"
		mkdir $dirname
		pushd $dirname
		RunRpcSelectionStrategies "${transmissionArgs} ${packetArgs} ${loadArgs}"
		popd
	done
}

function  RunDebug {
	debug=true
	echo "Running Debug"
	transmissionArgs=$(NormalClientTransmission 500000 50000)
	packetArgs=$(NormalPacketSizes 128 12)
	loadArgs=$(NormalServerLoad 50000 5000)
	#selectionArgs=$(RpcSelectionStrat single)
	selectionArgs=$(RpcSelectionStrat minimum)
	networkSelectionArgs=$(NetworkSelectionStrat minimum)
	configArgs=$(ConfigArgs results)
	currentdir=`pwd`
	dirArgs=$(WorkingDirectory $currentdir)

	args="${transmissionArgs} ${packetArgs} ${loadArgs} ${selectionArgs} ${networkSelectionArgs} ${configArgs} ${dirArgs}"

	pushd $topdir

	#./waf --visualize --command-template="gdb --args %s" --run "scratch/replication"
	#./waf --command-template="gdb %s" --run "scratch/replication" #"${args}" 
	./waf --run "scratch/replication ${args}" 

	mv results* $currentdir
	popd #topdir
	popd #debugdir

}

function PlotIntervalsExperiment() {
	dirname=`pwd`
	echo "Entering plotting in $dirname"
	for dir in ./*/     # list directories in the form "/tmp/dirname/"
	do
		dir=${dir%*/}      # remove the trailing "/"
		echo ${dir##*/}    # print everything after the final "/"
		pushd $dir
		#PlotInterval
		popd
	done

	wait

	pwd
	#collect aggregate measures
	files=""
	files=`find | grep results.dat`
	aggScript="$topdir/plot/library/percentile_latency.py"
	python $aggScript $files > aggregate.dat 

	plotScript="$topdir/plot/library/agg_latency.py"
	python $plotScript aggregate.dat &

	#collect switch aggregate measures
	files=""
	files=`find | grep RouterSummary.dat`
	aggScript="$topdir/plot/library/combine_switch_measurements.py"
	python $aggScript $files > aggregate_switch.dat

	plotScript="$topdir/plot/library/agg_switch_redirect.py"
	python $plotScript aggregate_switch.dat &

	wait

}

function PlotIntervalExperimentAverage {

	#files=("Server_Normal_Client_Normal_2020-05-06_1/aggregate.dat" "Server_Normal_Client_Normal_2020-05-06_2/aggregate.dat" "Server_Normal_Client_Normal_2020-05-06_3/aggregate.dat")
	files=$@
	#add file extension

	plotScript="$topdir/plot/library/avg_agg_latency.py"
	echo "Entering Python Plot"
	python $plotScript ${files[@]}
	CurrentDate=`date "+%F_%T"`
	cp Avg_Agg_Latency.pdf "Avg_Agg_Latency_${CurrentDate}.pdf"
	cp Avg_Agg_Latency.db "Avg_Agg_Latency_${CurrentDate}.db"

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
	-d=* | --dir=* )
	DIRECTORY="${i#*=}"
	;;
	--dirs=* )
	DIRECTORIES="${i#*=}"
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

if [ ! -z "$DEBUG" ]; then
	rm -r ./Experiments/debug
	mkdir ./Experiments/debug
	pushd ./Experiments/debug
	RunDebug
	popd
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
pushd $ExperimentDir


#determine what to run

#check to see if the function exists. This is largely for higher level scripting
if [ ! -z $FUNCTION ]; then
	if [ `type -t $FUNCTION`"" == 'function' ]; then
		echo "Running $FUNCTION"
		$FUNCTION
	else
		echo "$FUNCTION is not an in scope function: Exiting"
	fi
else 
	echo "running some defualt bulshit"
	#RunUniformTransmissionExperiment
	#RunNormalTransmissionExperiment
	#RunExponentialTransmissionExperiment
	#RunUniformPacketUniformTransmissionExperiment
	#RunNormalServerLoad
	#RunExponentialServerLoad
	RunProportionalLoad200
fi
#wait for any parallel tasks before exiting
wait
echo "Done Waiting Exiting $0"

popd
exit
