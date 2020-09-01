script="../../plot/library/multi_run_latency_comp.py"
exp="../../Experiments"
a="/VariedSkew-None/node"
b="/VariedSkew-TORQ/node"


function PlotHeatmap {
    percentile=$1
    delay=$2
    python $script $percentile "${exp}${a}/${delay}.db"  "${exp}${b}/${delay}.db"
    mv Heatmap_$percentile.pdf Heatmap-${delay}_${percentile}.pdf
}

percentiles=(50 95 99)
delays=(250 500 1000 2000 5000 10000)


for p in ${percentiles[@]}; do
    for d in ${delays[@]}; do
        PlotHeatmap $p $d
    done
done

