echo "Running Incast 1500B"

echo "Seer cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'
for sram in 16 32 48 64 80 96 112 128 144
do
    bin/driver -f trace_incast_144.csv.processed -a 1 -s $sram -i 1 -v 120 -x 5 -y 5 >> experiment_seermb_incast_1500.txt
done

echo "Running All-to-All 1500B"

for load in 0.3 0.6 0.9
do
    echo "Load $load"

    echo "Seer cases"
    for sram in 144 288 432 576 720 864 1008 1152 1296 1440 1584 1728 1872 2016 2160 2304 2448
    do
        bin/driver -f trace_all_to_all_144.csv.processed -a 1 -s $sram -i 1 -u 5 -l $load >> experiment_seermb_all_to_all_1500.txt
    done
done

echo "Running Permutation 1500B"

for load in 0.3 0.6 0.9
do
    echo "Load $load"

    echo "Seer cases"
    for sram in 2 4 6 8 10 12 14 16
    do
        bin/driver -f trace_permutation.csv.processed -a 1 -s $sram -i 1 -u 5 -l $load >> experiment_seermb_permutation_1500.txt
    done
done
