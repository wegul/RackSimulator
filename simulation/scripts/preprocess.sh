cd ./

for file in *.csv; do
    if [[ "$file" == mem*.csv ]]; then
        echo python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi "${file}" -ism 1 -fo "./proced-11-7/proced_${file} -b 100"
        python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi "${file}" -ism 1 -fo "./proced-11-7/proced_${file}"  -b 100
    else
        echo python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi "${file}" -ism 0 -fo "./proced-11-7/proced_${file} -b 100"
        python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi "${file}" -ism 0 -fo "./proced-11-7/proced_${file}"  -b 100
    fi
done

# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.2.csv -fo ./proced_memcached-trace-100G-0.2.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.4.csv -fo ./proced_memcached-trace-100G-0.4.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.6.csv -fo ./proced_memcached-trace-100G-0.6.csv -ism 1
# python3 ~/Desktop/RackSimulator/simulation/scripts/tracefile_preprocessor.py -fi ./memcached-trace-100G-0.8.csv -fo ./proced_memcached-trace-100G-0.8.csv -ism 1
