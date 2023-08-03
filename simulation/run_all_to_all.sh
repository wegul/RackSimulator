while getopts f:u:l:d: flag
do
    case "${flag}" in
        f) filename=${OPTARG};;
        u) burst=${OPTARG};;
        l) load=${OPTARG};;
        d) dram_at=${OPTARG};;
    esac
done

make clean
make all

bin/driver -f $filename -a 1 -s 2448 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 2304 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 2160 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 2016 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1872 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1728 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1584 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1440 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1296 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1152 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1008 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 864 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 720 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 576 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 432 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 288 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 144 -i 1 -u $burst -l $load

make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -s 2448 -i 1 -u $burst -l $load
rm ./out/trace_all_to_all_144.csv.processed.out
rm ./out/trace_all_to_all_144.csv.timeseries.csv
cp ./out/trace_all_to_all_144.csv.processed.spine0.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine0.csv
cp ./out/trace_all_to_all_144.csv.processed.spine1.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine1.csv
cp ./out/trace_all_to_all_144.csv.processed.spine2.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine2.csv
cp ./out/trace_all_to_all_144.csv.processed.spine3.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine3.csv
cp ./out/trace_all_to_all_144.csv.processed.spine4.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine4.csv
cp ./out/trace_all_to_all_144.csv.processed.spine5.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine5.csv
cp ./out/trace_all_to_all_144.csv.processed.spine6.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine6.csv
cp ./out/trace_all_to_all_144.csv.processed.spine7.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine7.csv
cp ./out/trace_all_to_all_144.csv.processed.spine8.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine8.csv
cp ./out/trace_all_to_all_144.csv.processed.spine9.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine9.csv
cp ./out/trace_all_to_all_144.csv.processed.spine10.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine10.csv
cp ./out/trace_all_to_all_144.csv.processed.spine11.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine11.csv
cp ./out/trace_all_to_all_144.csv.processed.spine12.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine12.csv
cp ./out/trace_all_to_all_144.csv.processed.spine13.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine13.csv
cp ./out/trace_all_to_all_144.csv.processed.spine14.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine14.csv
cp ./out/trace_all_to_all_144.csv.processed.spine15.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.spine15.csv
cp ./out/trace_all_to_all_144.csv.processed.tor0.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.tor0.csv
cp ./out/trace_all_to_all_144.csv.processed.tor1.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.tor1.csv
cp ./out/trace_all_to_all_144.csv.processed.tor2.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.tor2.csv
cp ./out/trace_all_to_all_144.csv.processed.tor3.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.tor3.csv
cp ./out/trace_all_to_all_144.csv.processed.tor4.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.tor4.csv
cp ./out/trace_all_to_all_144.csv.processed.tor5.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.tor5.csv
cp ./out/trace_all_to_all_144.csv.processed.tor6.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.tor6.csv
cp ./out/trace_all_to_all_144.csv.processed.tor7.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.tor7.csv
cp ./out/trace_all_to_all_144.csv.processed.tor8.csv ./out/all_to_all_512/trace_all_to_all_144.csv.processed.tor8.csv
python scripts/belady.py -f ./out/all_to_all_512 -n 2448 -x 2449 -s 144 -d $dram_at > best_case_all_to_all.txt
bin/driver -f $filename -a 1 -s 2304 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 2160 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 2016 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1872 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1728 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1584 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1440 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1296 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1152 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 1008 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 864 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 720 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 576 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 432 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 288 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 144 -i 1 -u $burst -l $load
