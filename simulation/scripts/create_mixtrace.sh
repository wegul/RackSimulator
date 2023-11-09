# Use this in RAW directory. It'll generate all ready-to-go traces

SCRIPT_DIR="$HOME/Desktop/RackSimulator/simulation/scripts"

if [[ "$#" < 2 ]]; then
    echo "Usage: $0 <output path name> <bandwidth>"
    exit 1
fi

outpath=$1
bw=$2
declare -a memratio=("0.1" "0.2" "0.3" "0.4" "0.6")
declare -a netratio=("0.1" "0.2" "0.3" "0.4" "0.6")

if [[ ! -d "$outpath" ]]; then
    echo "Directory created: mkdir -p $outpath"
    mkdir -p "$outpath"
fi

# preprocess to generate proced*
for file in *.csv; do
    if [[ $file == graphlab*.csv || $file == bdp*.csv || $file == memcached*.csv ]]; then
        echo "python3 ${SCRIPT_DIR}/tracefile_preprocessor.py -fi ${file} -fo ${outpath}/${file} -ism 1 -b ${bw}"
        python3 ${SCRIPT_DIR}/tracefile_preprocessor.py -fi ${file} -fo ${outpath}/proced_${file} -ism 1 -b ${bw}
    elif [[ $file != proced*.csv ]]; then
        echo "python3 ${SCRIPT_DIR}/tracefile_preprocessor.py -fi ${file} -fo ${outpath}/${file} -ism 0 -b ${bw}"
        python3 ${SCRIPT_DIR}/tracefile_preprocessor.py -fi ${file} -fo ${outpath}/proced_${file} -ism 0 -b ${bw}
    fi
done

#create mix*.csv
echo "cd $outpath"
cd $outpath

declare -a netloads=("dctcp" "aditya" "datamining")
declare -a memloads=("memcached" "graphlab" "bdp")

for memf in "${memloads[@]}"; do
    for netf in "${netloads[@]}"; do
        for memr in "${memratio[@]}"; do
            for netr in "${netratio[@]}"; do
                for memfile in "proced_${memf}"*"${memr}.csv"; do
                    if [[ -f $memfile ]]; then
                        for netfile in "proced_${netf}"*"${netr}.csv"; do
                            if [[ -f $netfile ]]; then
                                echo "cat ${memfile} ${netfile} >${outpath}/mix_${memf}${memr}_${netf}${netr}-100G.csv"
                                cat ${memfile} ${netfile} >${outpath}/mix_${memf}${memr}_${netf}${netr}-100G.csv
                            fi
                        done
                    fi
                done
            done
        done
    done
done

# sort
for file in mix*.csv; do
    if [[ -f $file ]]; then
        echo "python3 ${SCRIPT_DIR}/sort_by_starttime.py -fi ${outpath}/${file} -fo ${outpath}/${file}"
        python3 ${SCRIPT_DIR}/sort_by_starttime.py -fi ${outpath}/${file} -fo ${outpath}/${file}
    fi
done

if [[ ! -d ${outpath}/all-traces/ ]]; then
    echo mkdir -p "${outpath}/all-traces"
    mkdir -p "${outpath}/all-traces"
fi
echo ${SCRIPT_DIR}/copy_files.sh ${outpath}/all-traces/
${SCRIPT_DIR}/copy_files.sh ${outpath}/all-traces/
