#!/bin/bash

function usage {
    echo 'usage: ./run.sh -f <filename> [-w <0/1> -e <epochs> -b <link bandwidth> -c <pkt size> -h <header overhead> -d <propagation delay in ns> -n <num of flows> -m <num of flows>]'
    echo '-f: trace file path'
    echo '-w: 1 = static workload; 0 = dynamic workload'
    echo '-e: run the exp till specified num of epochs'
    echo '-b: link bandwidth (in Gbps)'
    echo '-c: packet size (in bytes)'
    echo '-h: header overhead (in bytes)'
    echo '-d: per hop propagation delay (in ns)'
    echo '-n: run the exp till specified num of flows finished'
    echo '-m: run the exp till specified num of flows started'
    exit 1
}

filepath=""
filename=""
static_workload=1
epochs="0"
bandwidth=100
pkt_size=64
header_overhead=0 #in bytes
prop_delay=0
numflowsfinish=5000
numflowsstart=5000

while getopts "f:w:e:b:c:h:d:n:m:rpa" OPTION
do
    case $OPTION in
        f) filepath=$OPTARG
            ;;
        w) static_workload=$OPTARG
            ;;
        e) epochs=$OPTARG
            ;;
        b) bandwidth=$OPTARG
            ;;
        c) pkt_size=$OPTARG
            ;;
        h) header_overhead=$OPTARG
            ;;
        d) prop_delay=$OPTARG
            ;;
        n) numflowsfinish=$OPTARG
            ;;
        m) numflowsstart=$OPTARG
            ;;
        *) usage
            ;;
    esac
done

if [ "$filepath" == "" ];
then
    usage
fi

filename=$(basename "${filepath}") #extract the filename from the path

echo -e "\n*** Running [./bin/driver -f "${filepath}" -w "${static_workload}" -e "${epochs}" -b "${bandwidth}" -c "${pkt_size}" -h "${header_overhead}" -d "${prop_delay}" -n "${numflowsfinish}" -m "${numflowsstart}"] ***\n"
./bin/driver -f "${filepath}" -w "${static_workload}" -e "${epochs}" -b "${bandwidth}" -c "${pkt_size}" -h "${header_overhead}" -d "${prop_delay}" -n "${numflowsfinish}" -m "${numflowsstart}"

