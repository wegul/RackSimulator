while getopts f:a:l:u: flag
do
    case "${flag}" in
        f) filename=${OPTARG};;
        l) load=${OPTARG};;
        u) burst=${OPTARG};;
    esac
done

make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -l $load -u $burst -s 800
bin/driver -f $filename -a 1 -l $load -u $burst -s 700
bin/driver -f $filename -a 1 -l $load -u $burst -s 600
bin/driver -f $filename -a 1 -l $load -u $burst -s 500
bin/driver -f $filename -a 1 -l $load -u $burst -s 400
bin/driver -f $filename -a 1 -l $load -u $burst -s 300
bin/driver -f $filename -a 1 -l $load -u $burst -s 200
bin/driver -f $filename -a 1 -l $load -u $burst -s 100