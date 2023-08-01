while getopts f:a:l:u: flag
do
    case "${flag}" in
        f) filename=${OPTARG};;
        l) load=${OPTARG};;
        u) burst=${OPTARG};;
    esac
done

make clean
make all

bin/driver -f $filename -a 1 -u $burst -s 10
bin/driver -f $filename -a 1 -u $burst -s 9
bin/driver -f $filename -a 1 -u $burst -s 8
bin/driver -f $filename -a 1 -u $burst -s 7
bin/driver -f $filename -a 1 -u $burst -s 6
bin/driver -f $filename -a 1 -u $burst -s 5
bin/driver -f $filename -a 1 -u $burst -s 4
bin/driver -f $filename -a 1 -u $burst -s 3
bin/driver -f $filename -a 1 -u $burst -s 2
bin/driver -f $filename -a 1 -u $burst -s 1

make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -u $burst -s 10
bin/driver -f $filename -a 1 -u $burst -s 9
bin/driver -f $filename -a 1 -u $burst -s 8
bin/driver -f $filename -a 1 -u $burst -s 7
bin/driver -f $filename -a 1 -u $burst -s 6
bin/driver -f $filename -a 1 -u $burst -s 5
bin/driver -f $filename -a 1 -u $burst -s 4
bin/driver -f $filename -a 1 -u $burst -s 3
bin/driver -f $filename -a 1 -u $burst -s 2
bin/driver -f $filename -a 1 -u $burst -s 1