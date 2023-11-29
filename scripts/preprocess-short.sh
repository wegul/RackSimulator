cd ./
# if [[ "$#" < 1 ]]; then
#     echo "Usage: $0 <output path name>"
#     exit 1
# fi
# outpath=$1

# if [[ ! -d $outpath ]]; then
#     echo "mkdir -p $outpath"
#     mkdir -p $outpath
# fi

# declare -a memPrefix=("bdp" "graphlab" "memcached" "terasortHadoop" "terasortSpark" "short")
# declare -a netPrefix=("aditya" "dctcp" "datamining")

for file in *.csv; do
    if [[ "$file" == *.csv ]]; then
        # for memName in "${memPrefix[@]}"; do
        # if [[ "${file}" == "${memName}"*.csv ]]; then
        #     # para_n=$(echo "${file#*100G-}" | grep -oE '[0-9]+')
        #     para_n=${file#*100G-}
        #     # echo $para_n
        #     # rreq
        #     echo "python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi ${file} -ism 1 -fo ../proced/rreq-proced_${memName}_${para_n} -msg 0"
        #     python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi ${file} -ism 1 -fo ../proced/rreq-proced_${memName}_${para_n} -msg 0
        #     echo "python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi ${file} -ism 0 -fo ../netVer/rreq-netVer_${memName}_${para_n} -msg 0"
        #     python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi ${file} -ism 0 -fo ../netVer/rreq-netVer_${memName}_${para_n} -msg 0
        #     # wreq
        #     echo "python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi ${file} -ism 1 -fo ../proced/wreq-proced_${memName}_${para_n} -msg 1"
        #     python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi ${file} -ism 1 -fo ../proced/wreq-proced_${memName}_${para_n} -msg 1
        #     echo "python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi ${file} -ism 0 -fo ../netVer/wreq-netVer_${memName}_${para_n} -msg 1"
        #     python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi ${file} -ism 0 -fo ../netVer/wreq-netVer_${memName}_${para_n} -msg 1

        # fi
        # done

        echo python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi "${file}" -ism 1 -fo "wreq_32B_${file} -msg 1"
        python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi "${file}" -ism 1 -fo "wreq_32B_${file}" -msg 1

        echo python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi "${file}" -ism 1 -fo "rreq_32B_${file} -msg 0"
        python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi "${file}" -ism 1 -fo rreq_32B_${file} -msg 0

    fi
done

# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.2.csv -fo ./proced_memcached-trace-100G-0.2.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.4.csv -fo ./proced_memcached-trace-100G-0.4.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.6.csv -fo ./proced_memcached-trace-100G-0.6.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.8.csv -fo ./proced_memcached-trace-100G-0.8.csv -ism 1
