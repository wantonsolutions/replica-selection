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


function runExperiment () {
	cpnp=$1
	cpi=$2
	cpps=$3
	cnp=$4
	ci=$5
	cps=$6

	selectionStrategy=$8
	incRate=$9
	filename=exp_$7_$1-$2-$3-$4-$5-$6-$8-$9

#./waf --visualize --run "scratch/replication"
#./waf --run "scratch/replication" --command-template="gdb %s"
	transmission=$(UniformClientTransmission 1000 1000)

./waf --run \
	"scratch/replication
	--SelectionStrategy=$selectionStrategy
	$transmission
	--Debug=$debug
    --ProbeName=$filename.csv
	--ManifestName=$filename.config" 2>$filename.dat
}


function RunInterval() {
	max=$1
	min=$2
	args=""
	transmission=$(UniformClientTransmission $min $max)
	args="$args 
	$transmission"

	for selection in "${!SelectionStrategy[@]}"; do
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


function RunAndMove() {
	echo "running and moving"
	runExperiment $1 $2 $3 $4 $5 $6 $7 $8 ${10}
	finalLoc=$9
	filename=exp_$7_$1-$2-$3-$4-$5-$6-$8-${10}
	mv $filename.dat $finalLoc.dat
	mv $filename.csv $finalLoc.csv
	mv $filename.config $finalLoc.config
}

function RunStaticIntervalExperiment() {
	intervals=(50000 25000 12500 6250)

	for i in ${intervals[@]}; do
		echo $i
		dirname="interval_${i}_us"
		mkdir $dirname
		pushd $dirname
		RunInterval $i $i
		popd
	done

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
	-h | --help)
	echo "No proper help message ready to go just read through the code for now"
esac

done

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

echo $1

#packetSize=1472
packetSize=64
totalPackets=10000
#packetSize=1000


if [[ $1 == "debug" ]];then
	echo "debugging"
	runExperiment 0 1.0 128 $totalPackets 1.0 $packetSize "debug" 0 0.9
	exit 0
elif [[ $1 == "single" ]];then
	echo "running a single run (likely for testing)"
    rate=0.99
    dataDir=queuelat
	RunAndMove 0 1.0 128 $totalPackets 1.0 $packetSize "single" 2 "data/$dataDir/single_$datetime" $rate
	ln -sf "single_$datetime.dat" "data/$dataDir/single_latest.dat"
	ln -sf "single_$datetime.csv" "data/$dataDir/single_latest.csv"
	ln -sf "single_$datetime.config" "data/$dataDir/single_latest.config"
else
	echo "testing replication strategy"
    rate=0.99
    dataDir=queuelat
	RunAndMove 0 1.0 128 $totalPackets 1.0 $packetSize "single" 0 "data/$dataDir/single_$datetime" $rate
	RunAndMove 0 1.0 128 $totalPackets 1.0 $packetSize "random" 1 "data/$dataDir/random_$datetime" $rate
	RunAndMove 0 1.0 128 $totalPackets 1.0 $packetSize "minimum" 2 "data/$dataDir/minimum_$datetime" $rate
	ln -sf "single_$datetime.dat" "data/$dataDir/single_latest.dat"
	ln -sf "random_$datetime.dat" "data/$dataDir/random_latest.dat"
	ln -sf "minimum_$datetime.dat" "data/$dataDir/minimum_latest.dat"
	ln -sf "single_$datetime.csv" "data/$dataDir/single_latest.csv"
	ln -sf "random_$datetime.csv" "data/$dataDir/random_latest.csv"
	ln -sf "minimum_$datetime.csv" "data/$dataDir/minimum_latest.csv"
	ln -sf "single_$datetime.config" "data/$dataDir/single_latest.config"
	ln -sf "random_$datetime.config" "data/$dataDir/random_latest.config"
	ln -sf "minimum_$datetime.config" "data/$dataDir/minimum_latest.config"

	cd plot/latqueue
	./plot.sh
	exit 0
fi

