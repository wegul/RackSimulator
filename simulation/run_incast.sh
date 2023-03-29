while getopts f:a:x:y:z: flag
do
    case "${flag}" in
        f) filename=${OPTARG};;
        x) change=${OPTARG};;
        y) duration=${OPTARG};;
        z) pause=${OPTARG};;
    esac
done

make clean
make all

bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 2222
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 1667
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 1111
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 444
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 333
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 222
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 111
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 22

bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 2222
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 1667
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 1111
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 444
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 333
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 222
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 111
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 22

make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 2222
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 1667
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 1111
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 444
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 333
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 222
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 111
bin/driver -f $filename -a 1 -x $change -y $duration -z $pause -s 22

bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 2222
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 1667
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 1111
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 444
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 333
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 222
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 111
bin/driver -f $filename -a 0 -x $change -y $duration -z $pause -s 22