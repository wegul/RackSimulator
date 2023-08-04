
echo "Running Aditya 64B"

echo "Load 0.3"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.3-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_03_64.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.3-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_03_64.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python3 scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 19 >> best_case_aditya_03_64.txt
    rm -r ./out/*.*
done

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_06_64.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_aditya_06_64.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python3 scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 19 >> best_case_aditya_06_64.txt
    rm -r ./out/*.*
done

echo "Load 0.9"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.9-aditya.csv.processed -a 1 -i 1 -u 5 -s $sram >> experiment_aditya_09_64.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.9-aditya.csv.processed -a 1 -i 1 -u 5 -s $sram >> experiment_aditya_09_64.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python3 scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 19 >> best_case_aditya_09_64.txt
    rm -r ./out/*.*
done

echo "Running Datamining 64B"

echo "Load 0.3"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.3-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_03_64.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.3-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_03_64.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python3 scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 19 >> best_case_datamining_03_64.txt
    rm -r ./out/*.*
done

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.6-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_06_64.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.6-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_06_64.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python3 scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 19 >> best_case_datamining_06_64.txt
    rm -r ./out/*.*
done

echo "Load 0.9"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.9-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_09_64.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.9-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_datamining_09_64.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python3 scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 19 >> best_case_datamining_09_64.txt
    rm -r ./out/*.*
done

echo "Running DCTCP 64B"

echo "Load 0.3"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.3-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_03_64.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.3-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_03_64.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python3 scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 19 >> best_case_dctcp_03_64.txt
    rm -r ./out/*.*
done

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.6-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_06_64.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.6-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_06_64.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python3 scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 19 >> best_case_dctcp_06_64.txt
    rm -r ./out/*.*
done

echo "Load 0.9"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.9-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_09_64.txt
done

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50
do
    bin/driver -f trace-100G-0.9-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_dctcp_09_64.txt
    rm -r ./out/*.out
    rm -r ./out/*.timeseries.csv
    python3 scripts/belady.py -f ./out -n $sram -x 51 -s 10000 -d 19 >> best_case_dctcp_09_64.txt
    rm -r ./out/*.*
done
