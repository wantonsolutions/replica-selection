#!/bin/bash -x

echo "Hello Experiment"

debug=false


function UniformClientTransmission() {
	min=$1
	max=$2
	echo "--TransmissionDistributionUniform=true 
	--TransmissionUniformDistributionMin=$1
	--TransmissionUniformDistributionMax=$2 "
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



function IncrementalIntervals() {
	intervals=(0.1 0.01 0.001 0.0001 0.00001 0.000001 0.0000001)
	ClientProtocolNPackets=4096
	ClientProtocolInterval=0.01
	ClientProtocolPacketSize=1024
	CoverNPackets=1000000
	CoverInterval=$intervals
	CoverPacketSize=1024
	let 'exp=0'
	for i in `seq  0.0000001 -0.00000001 0.00000001`; do
		runExperiment $ClientProtocolNPackets $ClientProtocolInterval $ClientProtocolPacketSize $CoverNPackets $i $CoverPacketSize $exp &
		let 'exp=exp+1'
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

datetime=`date "+%F_%T"`

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

