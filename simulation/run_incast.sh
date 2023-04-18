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
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 70
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 60
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 50
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 40
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 30
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 20
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 10

make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 80
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 70
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 60
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 50
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 40
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 30
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 20
bin/driver -f $filename -a 1 -v $active -x $change -y $duration -z $pause -s 10