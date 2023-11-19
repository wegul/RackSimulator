cd ./
if [[ "$#" < 1 ]]; then
    echo "Usage: $0 <output path name>"
    exit 1
fi
outpath=$1

if [[ ! -d $outpath ]]; then
    echo "mkdir -p $outpath"
    mkdir -p $outpath
fi

for file in *.csv; do
    if [[ "$file" == tera*.csv ]]; then
        echo python3 ~/Desktop/RackSimulator/scripts/tracefile_preprocessor.py -fi "${file}" -ism 1 -fo "${outpath}/proced_${file} -b 100"
        python3 ~/Desktop/RackSimulator/scripts/tracefile_preprocessor.py -fi "${file}" -ism 1 -fo "${outpath}/proced_${file}" -b 100
    else
        echo python3 ~/Desktop/RackSimulator/scripts/tracefile_preprocessor.py -fi "${file}" -ism 0 -fo "${outpath}/proced_${file} -b 100"
        # python3 ~/Desktop/RackSimulator/scripts/tracefile_preprocessor.py -fi "${file}" -ism 0 -fo "${outpath}/proced_${file}" -b 100
    fi
done

# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.2.csv -fo ./proced_memcached-trace-100G-0.2.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.4.csv -fo ./proced_memcached-trace-100G-0.4.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.6.csv -fo ./proced_memcached-trace-100G-0.6.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.8.csv -fo ./proced_memcached-trace-100G-0.8.csv -ism 1
