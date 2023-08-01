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
