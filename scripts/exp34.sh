TRACE_PATH="${HOME}/Desktop/RackSimulator/shortflow-exp34/"
BIN_PATH="${HOME}/Desktop/RackSimulator/simulation/bin/"
SCRPT_PATH="${HOME}/Desktop/RackSimulator/scripts"
# echo "mkdir -p ./memtrace-load0.6/"
# mkdir -p ./memtrace-load0.6/

cd ./

declare -a memPrefix=("rreq" "wreq")
declare -a algoMem=("batch" "ndp" "fastpass")
declare -a algoNet=("dctcp" "pfc" "pfabric")

for file in ${TRACE_PATH}*.csv; do
    file=${file#${TRACE_PATH}*}
    for memName in ${memPrefix[@]}; do
        if [[ $file == ${memName}*.csv ]]; then # get memfile
            # if [[ $file == rreq* ]]; then #run IDEAL SIMU
            #     echo "python3 ${SCRPT_PATH}/IdealSimulatorRREQ.py -fi ${file} -fo ideal-${file}"
            #     python3 ${SCRPT_PATH}/IdealSimulatorRREQ.py -fi ${file} -fo ideal-${file}
            # fi
            # if [[ $file == wreq* ]]; then
            #     echo "python3 ${SCRPT_PATH}/IdealSimulatorWREQ.py -fi ${file} -fo ideal-${file}"
            #     python3 ${SCRPT_PATH}/IdealSimulatorWREQ.py -fi ${file} -fo ideal-${file}
            # fi
            for program in ${algoMem[@]}; do
                # echo "cp ${file} ./${program}-${file}"
                # cp ${file} ./${program}-${file}
                echo "screen -dm bash -c \"${BIN_PATH}driver-${program} -f ./${program}-${file} >log-${program}-${file}.txt\""
                screen -dm bash -c "${BIN_PATH}driver-${program} -f ./${program}-${file} >log-${program}-${file}.txt"
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
