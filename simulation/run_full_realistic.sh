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

bin/driver -f $filename -a 1 -u $burst -s 5000
bin/driver -f $filename -a 1 -u $burst -s 4000
bin/driver -f $filename -a 1 -u $burst -s 3000
bin/driver -f $filename -a 1 -u $burst -s 2000
bin/driver -f $filename -a 1 -u $burst -s 1000
bin/driver -f $filename -a 1 -u $burst -s 500
bin/driver -f $filename -a 1 -u $burst -s 250
bin/driver -f $filename -a 1 -u $burst -s 100

make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -u $burst -s 5000
bin/driver -f $filename -a 1 -u $burst -s 4000
bin/driver -f $filename -a 1 -u $burst -s 3000
bin/driver -f $filename -a 1 -u $burst -s 2000
bin/driver -f $filename -a 1 -u $burst -s 1000
bin/driver -f $filename -a 1 -u $burst -s 500
bin/driver -f $filename -a 1 -u $burst -s 250
bin/driver -f $filename -a 1 -u $burst -s 100