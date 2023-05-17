while getopts f:v:x:y:z: flag
do
    case "${flag}" in
        f) filename=${OPTARG};;
        v) active=${OPTARG};;
        x) change=${OPTARG};;
        y) duration=${OPTARG};;
        z) pause=${OPTARG};;
    esac
done

make clean
make all

bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 80
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 72
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 64
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 56
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 48
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 40
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 32
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 24
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 16
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 8

make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 80
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 72
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 64
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 56
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 48
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 40
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 32
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 24
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 16
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 8