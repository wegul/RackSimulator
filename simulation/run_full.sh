while getopts f:a:l: flag
do
    case "${flag}" in
        f) filename=${OPTARG};;
        l) load=${OPTARG};;
    esac
done

make clean
make all

bin/driver -f $filename -a 1 -l $load -s 2222
bin/driver -f $filename -a 1 -l $load -s 1667
bin/driver -f $filename -a 1 -l $load -s 1111
bin/driver -f $filename -a 1 -l $load -s 444
bin/driver -f $filename -a 1 -l $load -s 333
bin/driver -f $filename -a 1 -l $load -s 222
bin/driver -f $filename -a 1 -l $load -s 111
bin/driver -f $filename -a 1 -l $load -s 22

bin/driver -f $filename -a 0 -l $load -s 2222
bin/driver -f $filename -a 0 -l $load -s 1667
bin/driver -f $filename -a 0 -l $load -s 1111
bin/driver -f $filename -a 0 -l $load -s 444
bin/driver -f $filename -a 0 -l $load -s 333
bin/driver -f $filename -a 0 -l $load -s 222
bin/driver -f $filename -a 0 -l $load -s 111
bin/driver -f $filename -a 0 -l $load -s 22

make clean
make INCLUDE_SNAPSHOTS='"TRUE"'

bin/driver -f $filename -a 1 -l $load -s 2222
bin/driver -f $filename -a 1 -l $load -s 1667
bin/driver -f $filename -a 1 -l $load -s 1111
bin/driver -f $filename -a 1 -l $load -s 444
bin/driver -f $filename -a 1 -l $load -s 333
bin/driver -f $filename -a 1 -l $load -s 222
bin/driver -f $filename -a 1 -l $load -s 111
bin/driver -f $filename -a 1 -l $load -s 22

bin/driver -f $filename -a 0 -l $load -s 2222
bin/driver -f $filename -a 0 -l $load -s 1667
bin/driver -f $filename -a 0 -l $load -s 1111
bin/driver -f $filename -a 0 -l $load -s 444
bin/driver -f $filename -a 0 -l $load -s 333
bin/driver -f $filename -a 0 -l $load -s 222
bin/driver -f $filename -a 0 -l $load -s 111
bin/driver -f $filename -a 0 -l $load -s 22