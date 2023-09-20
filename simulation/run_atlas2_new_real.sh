echo "Running Aditya 128B"

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-aditya-128B.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_06_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-aditya-128B.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_06_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 11 -s 10000 -d 9 >> best_case_aditya_06_128.txt
    rm -r ./out/*.*
done

echo "Running Datamining 128B"

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-datamining-128B.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_06_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-datamining-128B.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_06_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 11 -s 10000 -d 9 >> best_case_datamining_06_128.txt
    rm -r ./out/*.*
done