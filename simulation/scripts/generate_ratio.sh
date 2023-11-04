python3 ./scripts/select.py -fm ./proced_workloads/memcached_proced.csv -fn ./proced_workloads/dcload-1_proced.csv -x 3500 -fo mn1-9.csv -r 9
python3 ./scripts/select.py -fm ./proced_workloads/memcached_proced.csv -fn ./proced_workloads/dcload-1_proced.csv -x 4800 -fo mn3-7.csv -r 2.33
python3 ./scripts/select.py -fm ./proced_workloads/memcached_proced.csv -fn ./proced_workloads/dcload-1_proced.csv -x 5400 -fo mn5-5.csv -r 1
python3 ./scripts/select.py -fm ./proced_workloads/memcached_proced.csv -fn ./proced_workloads/dcload-1_proced.csv -x 6100 -fo mn7-3.csv -r 0.43
python3 ./scripts/select.py -fm ./proced_workloads/memcached_proced.csv -fn ./proced_workloads/dcload-1_proced.csv -x 6500 -fo mn9-1.csv -r 0.11

python3 ./scripts/shuffle.py -fi ./mn1-9.csv -fo ./mn1-9_shuffle.csv
python3 ./scripts/shuffle.py -fi ./mn3-7.csv -fo ./mn3-7_shuffle.csv
python3 ./scripts/shuffle.py -fi ./mn5-5.csv -fo ./mn5-5_shuffle.csv
python3 ./scripts/shuffle.py -fi ./mn7-3.csv -fo ./mn7-3_shuffle.csv
python3 ./scripts/shuffle.py -fi ./mn9-1.csv -fo ./mn9-1_shuffle.csv