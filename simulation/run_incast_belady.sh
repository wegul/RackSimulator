while getopts f:v:x:y:z: flag
do
    case "${flag}" in
        f) filename=${OPTARG};;
        v) active=${OPTARG};;
        x) change=${OPTARG};;
        y) duration=${OPTARG};;
    esac
done


make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -s 144 -i 1 -v $active -x $change -y $duration
python scripts/belady.py -f ./out -n 144 -x 145 -s 16 > best_case_incast.txt
bin/driver -f $filename -a 1 -s 128 -i 1 -v $active -x $change -y $duration
python scripts/belady.py -f ./out -n 128 -x 129 -s 16 >> best_case_incast.txt
bin/driver -f $filename -a 1 -s 112 -i 1 -v $active -x $change -y $duration
python scripts/belady.py -f ./out -n 112 -x 113 -s 16 >> best_case_incast.txt
bin/driver -f $filename -a 1 -s 96 -i 1 -v $active -x $change -y $duration
python scripts/belady.py -f ./out -n 96 -x 97 -s 16 >> best_case_incast.txt
bin/driver -f $filename -a 1 -s 80 -i 1 -v $active -x $change -y $duration
python scripts/belady.py -f ./out -n 80 -x 81 -s 16 >> best_case_incast.txt
bin/driver -f $filename -a 1 -s 64 -i 1 -v $active -x $change -y $duration
python scripts/belady.py -f ./out -n 64 -x 65 -s 16 >> best_case_incast.txt
bin/driver -f $filename -a 1 -s 48 -i 1 -v $active -x $change -y $duration
python scripts/belady.py -f ./out -n 48 -x 49 -s 16 >> best_case_incast.txt
bin/driver -f $filename -a 1 -s 32 -i 1 -v $active -x $change -y $duration
python scripts/belady.py -f ./out -n 32 -x 33 -s 16 >> best_case_incast.txt
bin/driver -f $filename -a 1 -s 16 -i 1 -v $active -x $change -y $duration
python scripts/belady.py -f ./out -n 16 -x 17 -s 16 >> best_case_incast.txt