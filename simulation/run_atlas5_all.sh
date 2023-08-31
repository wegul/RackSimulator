echo "Running Incast 1500B"

echo "Baseline cases"
make clean
make all

for sram in 16 32 48 64 80 96 112 128 144
do
    bin/driver -f trace_incast_144.csv.processed -a 1 -s $sram -i 1 -v 120 -x 5 -y 5 >> experiment_memory_incast_1500.txt
    bin/driver -f trace_incast_144.csv.processed -a 2 -s $sram -i 1 -v 120 -x 5 -y 5 >> experiment_memory_incast_1500.txt
    bin/driver -f trace_incast_144.csv.processed -a 3 -s $sram -i 1 -v 120 -x 5 -y 5 >> experiment_memory_incast_1500.txt
    bin/driver -f trace_incast_144.csv.processed -a 4 -s $sram -i 1 -v 120 -x 5 -y 5 >> experiment_memory_incast_1500.txt
done

echo "Running All-to-All 1500B"

for load in 0.6
do
    echo "Load $load"

    echo "Baseline cases"
    make clean
    make all
    for sram in 144 288 432 576 720 864 1008 1152 1296 1440 1584 1728 1872 2016 2160 2304 2448
    do
        bin/driver -f trace_all_to_all_144.csv.processed -a 1 -s $sram -i 1 -u 5 -l $load >> experiment_memory_all_to_all_1500.txt
        bin/driver -f trace_all_to_all_144.csv.processed -a 2 -s $sram -i 1 -u 5 -l $load >> experiment_memory_all_to_all_1500.txt
        bin/driver -f trace_all_to_all_144.csv.processed -a 3 -s $sram -i 1 -u 5 -l $load >> experiment_memory_all_to_all_1500.txt
        bin/driver -f trace_all_to_all_144.csv.processed -a 4 -s $sram -i 1 -u 5 -l $load >> experiment_memory_all_to_all_1500.txt
    done
done

echo "Running Permutation 1500B"

for load in 0.6
do
    echo "Load $load"

    echo "Baseline cases"
    make clean
    make all
    for sram in 2 4 6 8 10 12 14 16
    do
        bin/driver -f trace_permutation_144.csv.processed -a 1 -s $sram -i 1 -u 5 -l $load >> experiment_memory_permutation_1500.txt
        bin/driver -f trace_permutation_144.csv.processed -a 2 -s $sram -i 1 -u 5 -l $load >> experiment_memory_permutation_1500.txt
        bin/driver -f trace_permutation_144.csv.processed -a 3 -s $sram -i 1 -u 5 -l $load >> experiment_memory_permutation_1500.txt
        bin/driver -f trace_permutation_144.csv.processed -a 4 -s $sram -i 1 -u 5 -l $load >> experiment_memory_permutation_1500.txt
    done
done

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-aditya.csv.processed -a 1 -u 5 -s $sram >> experiment_memory_aditya_06_1500.txt
    bin/driver -f trace-100G-0.6-aditya.csv.processed -a 2 -u 5 -s $sram >> experiment_memory_aditya_06_1500.txt
    bin/driver -f trace-100G-0.6-aditya.csv.processed -a 3 -u 5 -s $sram >> experiment_memory_aditya_06_1500.txt
    bin/driver -f trace-100G-0.6-aditya.csv.processed -a 4 -u 5 -s $sram >> experiment_memory_aditya_06_1500.txt
done

echo "Running Datamining 1500B"
echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-datamining.csv.processed -a 1 -u 5 -s $sram >> experiment_memory_datamining_06_1500.txt
    bin/driver -f trace-100G-0.6-datamining.csv.processed -a 2 -u 5 -s $sram >> experiment_memory_datamining_06_1500.txt
    bin/driver -f trace-100G-0.6-datamining.csv.processed -a 3 -u 5 -s $sram >> experiment_memory_datamining_06_1500.txt
    bin/driver -f trace-100G-0.6-datamining.csv.processed -a 4 -u 5 -s $sram >> experiment_memory_datamining_06_1500.txt
done

echo "Load 0.9"

echo "Running DCTCP 1500B"

echo "Load 0.6"

echo "Baseline cases"
make clean
make all
for sram in 1 2 3 4 5 6 7 8 9 10
do
    bin/driver -f trace-100G-0.6-dctcp.csv.processed -a 1 -u 5 -s $sram >> experiment_memory_dctcp_06_1500.txt
    bin/driver -f trace-100G-0.6-dctcp.csv.processed -a 2 -u 5 -s $sram >> experiment_memory_dctcp_06_1500.txt
    bin/driver -f trace-100G-0.6-dctcp.csv.processed -a 3 -u 5 -s $sram >> experiment_memory_dctcp_06_1500.txt
    bin/driver -f trace-100G-0.6-dctcp.csv.processed -a 4 -u 5 -s $sram >> experiment_memory_dctcp_06_1500.txt
done
