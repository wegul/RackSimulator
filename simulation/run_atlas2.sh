echo "Running Incast 128B"

echo "Baseline cases"
make clean
make all

for sram in 16 32 48 64 80 96 112 128 144
do
    bin/driver -f trace_incast_144.csv.processed -a 1 -s $sram -i 1 -v 120 -x 5 -y 5 >> experiment_incast_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 16 32 48 64 80 96 112 128 144
do
    bin/driver -f trace_incast_144.csv.processed -a 1 -s $sram -i 1 -v 120 -x 5 -y 5 >> experiment_incast_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 145 -s 1000 -d 9 >> best_case_incast_128.txt
    rm -r ./out/*.*
done

echo "Running All-to-All 128B"

for load in 0.3 0.6 0.9
do
    echo "Load $load"

    echo "Baseline cases"
    make clean
    make all
    for sram in 144 288 432 576 720 864 1008 1152 1296 1440 1584 1728 1872 2016 2160 2304 2448
    do
        bin/driver -f trace_all_to_all_144.csv.processed -a 1 -s $sram -i 1 -u 5 -l $load >> experiment_all_to_all_128.txt
    done

    echo "Seer and Belady cases"
    make clean
    make INCLUDE_SNAPSHOTS='"TRUE"'
    for sram in 144 288 432 576 720 864 1008 1152 1296 1440 1584 1728 1872 2016 2160 2304 2448
    do
        bin/driver -f trace_all_to_all_144.csv.processed -a 1 -s $sram -i 1 -u 5 -l $load >> experiment_all_to_all_128.txt
        rm -r ./out/*.out
        rm -r ./out/*.timeseries.csv
        python scripts/belady.py -f ./out -n $sram -x 2449 -s 10000 -d 9 >> best_case_all_to_all_128.txt
        rm -r ./out/*.*
    done
done

echo "Running Permutation 128B"

for load in 0.3 0.6 0.9
do
    echo "Load $load"

    echo "Baseline cases"
    make clean
    make all
    for sram in 2 4 6 8 10 12 14 16
    do
        bin/driver -f trace_permutation_144.csv.processed -a 1 -s $sram -i 1 -u 5 -l $load >> experiment_permutation_128.txt
    done

    echo "Seer and Belady cases"
    make clean
    make INCLUDE_SNAPSHOTS='"TRUE"'
    for sram in 2 4 6 8 10 12 14 16
    do
        bin/driver -f trace_permutation.csv.processed -a 1 -s $sram -i 1 -u 5 -l $load >> experiment_permutation_128.txt
        rm -r ./out/*.out
        rm -r ./out/*.timeseries.csv
        python scripts/belady.py -f ./out -n $sram -x 17 -s 10000 -d 9 >> best_case_permutation_128.txt
        rm -r ./out/*.*
    done
done

echo "Running Aditya 128B"

echo "Load 0.3"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.3-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_03_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.3-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_03_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 9 >> best_case_aditya_03_128.txt
    rm -r ./out/*.*
done

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.6-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_06_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.6-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_06_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 9 >> best_case_aditya_06_128.txt
    rm -r ./out/*.*
done

echo "Load 0.9"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.9-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_09_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.9-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_09_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 9 >> best_case_aditya_09_128.txt
    rm -r ./out/*.*
done

echo "Running Datamining 128B"

echo "Load 0.3"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.3-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_03_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.3-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_03_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 9 >> best_case_datamining_03_128.txt
    rm -r ./out/*.*
done

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.6-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_06_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.6-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_06_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 9 >> best_case_datamining_06_128.txt
    rm -r ./out/*.*
done

echo "Load 0.9"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.9-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_09_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.9-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_09_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 9 >> best_case_datamining_09_128.txt
    rm -r ./out/*.*
done

echo "Running DCTCP 128B"

echo "Load 0.3"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.3-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_03_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.3-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_03_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 9 >> best_case_dctcp_03_128.txt
    rm -r ./out/*.*
done

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.6-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_06_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.6-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_06_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 9 >> best_case_dctcp_06_128.txt
    rm -r ./out/*.*
done

echo "Load 0.9"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.9-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_09_128.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.9-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_09_128.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 9 >> best_case_dctcp_09_128.txt
    rm -r ./out/*.*
done
