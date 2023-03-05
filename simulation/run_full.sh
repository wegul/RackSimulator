while getopts f:a:l: flag
do
    case "${flag}" in
        f) filename=${OPTARG};;
        a) associative=${OPTARG};;
        l) load=${OPTARG};;
    esac
done

bin/driver -f $filename -a $associative -l $load -s 2222
bin/driver -f $filename -a $associative -l $load -s 1667
bin/driver -f $filename -a $associative -l $load -s 1111
bin/driver -f $filename -a $associative -l $load -s 444
bin/driver -f $filename -a $associative -l $load -s 333
bin/driver -f $filename -a $associative -l $load -s 222
bin/driver -f $filename -a $associative -l $load -s 111
bin/driver -f $filename -a $associative -l $load -s 22