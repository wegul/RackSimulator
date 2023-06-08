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
make all

bin/driver -f $filename -a 1 -s 144 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 128 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 112 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 96 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 80 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 64 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 48 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 32 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 16 -i 1 -v $active -x $change -y $duration

make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -s 144 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 128 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 112 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 96 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 80 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 64 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 48 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 32 -i 1 -v $active -x $change -y $duration
bin/driver -f $filename -a 1 -s 16 -i 1 -v $active -x $change -y $duration
