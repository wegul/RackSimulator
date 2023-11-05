cd ./

python3 ./scripts/select.py -fm ./proced_workloads/memcached_proced.csv -fn ./proced_workloads/dcload-1_proced.csv -x 1500 -fo mn1-9.csv -r 9
python3 ./scripts/select.py -fm ./proced_workloads/memcached_proced.csv -fn ./proced_workloads/dcload-1_proced.csv -x 2800 -fo mn3-7.csv -r 2.33
python3 ./scripts/select.py -fm ./proced_workloads/memcached_proced.csv -fn ./proced_workloads/dcload-1_proced.csv -x 3400 -fo mn5-5.csv -r 1
python3 ./scripts/select.py -fm ./proced_workloads/memcached_proced.csv -fn ./proced_workloads/dcload-1_proced.csv -x 4100 -fo mn7-3.csv -r 0.43
python3 ./scripts/select.py -fm ./proced_workloads/memcached_proced.csv -fn ./proced_workloads/dcload-1_proced.csv -x 4500 -fo mn9-1.csv -r 0.11

python3 ./scripts/shuffle.py -fi ./mn1-9.csv -fo ./mn1-9_full.csv
python3 ./scripts/shuffle.py -fi ./mn3-7.csv -fo ./mn3-7_full.csv
python3 ./scripts/shuffle.py -fi ./mn5-5.csv -fo ./mn5-5_full.csv
python3 ./scripts/shuffle.py -fi ./mn7-3.csv -fo ./mn7-3_full.csv
python3 ./scripts/shuffle.py -fi ./mn9-1.csv -fo ./mn9-1_full.csv

for file in *full.csv; do
    if [ -f "$file" ]; then
        echo "python3 ./scripts/separate_mn.py -f $file"
        python3 ./scripts/separate_mn.py -f $file
        echo "python3 ./scripts/reverse_mem.py -fi $file -fo uni_$file"
        python3 ./scripts/reverse_mem.py -fi $file -fo uni_$file
    fi
done
