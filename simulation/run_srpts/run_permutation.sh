while getopts f:u:l: flag
do
    case "${flag}" in
        f) filename=${OPTARG};;
        u) burst=${OPTARG};;
        l) load=${OPTARG};;
    esac
done

make clean
make all

bin/driver -f $filename -a 1 -s 16 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 14 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 12 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 10 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 8 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 6 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 4 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 2 -i 1 -u $burst -l $load

make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -s 16 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 14 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 12 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 10 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 8 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 6 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 4 -i 1 -u $burst -l $load
bin/driver -f $filename -a 1 -s 2 -i 1 -u $burst -l $load