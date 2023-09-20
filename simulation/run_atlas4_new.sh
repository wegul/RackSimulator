echo "Running Incast 512B"

echo "Baseline cases"
make clean
make all

for sram in 16 32 48 64 80 96 112 128 144
do
    bin/driver -f trace_incast_144_512B.csv.processed -a 1 -s $sram -i 1 -v 120 -x 5 -y 5 >> experiment_incast_512.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 16 32 48 64 80 96 112 128 144
do
    bin/driver -f trace_incast_144_512B.csv.processed -a 1 -s $sram -i 1 -v 120 -x 5 -y 5 >> experiment_incast_512.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 145 -s 1000 -d 3 >> best_case_incast_512.txt
    rm -r ./out/*.*
done

echo "Running Permutation 512B"

for load in 0.6
do
    echo "Load $load"

    echo "Baseline cases"
    make clean
    make all
    for sram in 2 4 6 8 10 12 14 16 18 20
    do
        bin/driver -f trace_permutation_144_512B.csv.processed -a 1 -s $sram -i 1 -u 5 -l $load >> experiment_permutation_512.txt
    done

    echo "Seer and Belady cases"
    make clean
    make INCLUDE_SNAPSHOTS='"TRUE"'
    for sram in 2 4 6 8 10 12 14 16 18 20
    do
        bin/driver -f trace_permutation_144_512B.csv.processed -a 1 -s $sram -i 1 -u 5 -l $load >> experiment_permutation_512.txt
        rm -r ./out/*.out
        rm -r ./out/*.timeseries.csv
        python scripts/belady.py -f ./out -n $sram -x 21 -s 10000 -d 3 >> best_case_permutation_512.txt
        rm -r ./out/*.*
    done
done

echo "Running Aditya 512B"

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-aditya-512B.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_06_512.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-aditya-512B.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_06_512.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 11 -s 10000 -d 3 >> best_case_aditya_06_512.txt
    rm -r ./out/*.*
done

echo "Running Datamining 512B"

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-datamining-512B.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_06_512.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-datamining-512B.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_06_512.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 11 -s 10000 -d 3 >> best_case_datamining_06_512.txt
    rm -r ./out/*.*
done
