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

declare -a memPrefix=("bdp" "graphlab" "memcached" "terasortHadoop" "terasortSpark")
declare -a netPrefix=("aditya" "dctcp" "datamining")

for file in *.csv; do
    if [[ "$file" == *.csv ]]; then
        for memName in "${memPrefix[@]}"; do
            if [[ "${file}" == "${memName}"*.csv ]]; then
                # para_n=$(echo "${file#*100G-}" | grep -oE '[0-9]+')
                para_n=${file#*100G-}
                # echo $para_n
                # echo "python3 ~/Desktop/RackSimulator/scripts/tracefile_preprocessor.py -fi ${file} -ism 1 -fo ../proced/proced_${memName}_${para_n} -b 100"
                # python3 ~/Desktop/RackSimulator/scripts/tracefile_preprocessor.py -fi ${file} -ism 1 -fo ../proced/proced_${memName}_${para_n} -b 100

                # echo "python3 ~/Desktop/RackSimulator/scripts/convert_proced_to_net.py -fi ../proced/proced_${memName}_${para_n} -fo ../netVer/netVer_${memName}_${para_n}"
                # python3 ~/Desktop/RackSimulator/scripts/convert_proced_to_net.py -fi ../proced/proced_${memName}_${para_n} -fo ../netVer/netVer_${memName}_${para_n}
                echo "python3 ~/Desktop/RackSimulator/scripts/tracefile_preprocessor.py -fi ${file} -ism 0 -fo ../netVer/netVer_${memName}_${para_n} -b 100"
                python3 ~/Desktop/RackSimulator/scripts/tracefile_preprocessor.py -fi ${file} -ism 0 -fo ../netVer/netVer_${memName}_${para_n} -b 100
            fi
        done

        # echo python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi "${file}" -ism 1 -fo "${outpath}/proced_8B_${file} -b 100"
        # python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi "${file}" -ism 1 -fo "${outpath}/proced_8B_${file}" -b 100
        # # else
        # echo python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi "${file}" -ism 0 -fo "${outpath}/netVer_8B_${file} -b 100"
        # python3 ~/Desktop/RackSimulator/scripts/preprocess-short-flow.py -fi "${file}" -ism 0 -fo "${outpath}/netVer_8B_${file}" -b 100

    fi
done

# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.2.csv -fo ./proced_memcached-trace-100G-0.2.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.4.csv -fo ./proced_memcached-trace-100G-0.4.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.6.csv -fo ./proced_memcached-trace-100G-0.6.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.8.csv -fo ./proced_memcached-trace-100G-0.8.csv -ism 1
