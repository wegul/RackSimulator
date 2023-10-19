
echo "Baseline cases"
make clean
make all

bin/driver -f trace-100G-0.6-datamining.csv.processed -a 1 -u 5 -s 2 > experiment_datamining_128.txt

echo "Seer and Belady cases"
make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f trace-100G-0.6-datamining.csv.processed -a 1 -u 5 -s 2 >> experiment_datamining_128.txt

rm -r ./out/*.out
rm -r ./out/*.timeseries.csv
python scripts/belady.py -f ./out -n 2 -x 10 -s 10000 -d 9 > best_case_datamining_128.txt
rm -r ./out/*.*