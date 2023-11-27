TRACE_PATH="${HOME}/Desktop/RackSimulator/FInal-Traces/raw/"
BIN_PATH="${HOME}/Desktop/RackSimulator/simulation/bin/"
# echo "mkdir -p ./memtrace-load0.6/"
# mkdir -p ./memtrace-load0.6/

cd ./

declare -a memPrefix=("bdp" "graphlab" "memcached" "terasortHadoop" "terasortSpark")
declare -a algoMem=("edm" "ndp" "fastpass")
declare -a algoNet=("dctcp" "pfc" "pfabric")

for file in ${TRACE_PATH}*.csv; do
    for memName in ${memPrefix[@]}; do
        if [[ $file == *${memName}*0.6.csv ]]; then # get memfile
            para_n=${file#*100G-}
            procedGeneralName="./proced_${memName}-${para_n}"
            netVerGeneralName="./netVer_${memName}-${para_n}"
            echo "python3 ~/Desktop/RackSimulator/scripts/tracefile_preprocessor.py -fi ${file} -ism 1 -fo ${procedGeneralName} -b 100"
            python3 ~/Desktop/RackSimulator/scripts/tracefile_preprocessor.py -fi ${file} -ism 1 -fo ${procedGeneralName} -b 100
            # echo "python3 ~/Desktop/RackSimulator/scripts/convert_proced_to_net.py -fi ${procedGeneralName} -fo ${netVerGeneralName}"
            # python3 ~/Desktop/RackSimulator/scripts/convert_proced_to_net.py -fi ${procedGeneralName} -fo ${netVerGeneralName}
            for program in ${algoMem[@]}; do
                echo "cp ${procedGeneralName} ./${program}-proced_${memName}_${para_n}"
                cp ${procedGeneralName} ./${program}-proced_${memName}_${para_n}
                echo "screen -dm bash -c \"${BIN_PATH}driver-${program} -f ./${program}-proced_${memName}_${para_n} >${program}-log-${memName}_${para_n}.txt\""
                screen -dm bash -c "${BIN_PATH}driver-${program} -f ./${program}-proced_${memName}_${para_n} > ${program}-log-${memName}_${para_n}.txt"
            done
            # for program in ${algoNet[@]}; do
            #     echo "cp ${netVerGeneralName} ./${program}-netVer_${memName}_${para_n}"
            #     cp ${netVerGeneralName} ./${program}-netVer_${memName}_${para_n}
            #     echo "screen -dm bash -c \"${BIN_PATH}driver-${program} -f ./${program}-netVer_${memName}_${para_n} >${program}-log-${memName}_${para_n}.txt\""
            #     screen -dm bash -c "${BIN_PATH}driver-${program} -f ./${program}-netVer_${memName}_${para_n} > ${program}-log-${memName}_${para_n}.txt"
            # done
        fi
    done
done
