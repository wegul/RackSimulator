#!/bin/bash

function usage {
    echo 'usage: ./run.sh -f <filename> [-b <link bandwidth> -c <pkt size> -h <header overhead> -d <propagation delay in ns> -n <num of flows> -m <num of flows> -t <max_timeslots>]'
    echo '-f: trace file path'
    echo '-b: link bandwidth (in Gbps)'
    echo '-c: packet size (in bytes)'
    echo '-h: header overhead (in bytes)'
    echo '-d: per hop propagation delay (in ns)'
    echo '-n: run the exp till specified num of flows finished'
    echo '-m: run the exp till specified num of flows started'
    echo '-t: maximum number of timeslots to run'

    exit 1
}

filepath=""
filename=""
bandwidth=10
pkt_size=1500
header_overhead=0 #in bytes
prop_delay=0
numflowsfinish=5000
numflowsstart=5000
maxtimeslots=10000000
numdatapoints=1000

while getopts "f:b:c:h:d:n:m:t:q:rpa" OPTION
do
    case $OPTION in
        f) filepath=$OPTARG
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
        t) maxtimeslots=$OPTARG
            ;;
        q) numdatapoints=$OPTARG
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

echo -e "\n*** Running [./bin/driver -f "${filepath}" -b "${bandwidth}" -c "${pkt_size}" -h "${header_overhead}" -d "${prop_delay}" -n "${numflowsfinish}" -m "${numflowsstart}" -t "${maxtimeslots}" -q "${numdatapoints}"] ***\n"
./bin/driver -f "${filepath}" -b "${bandwidth}" -c "${pkt_size}" -h "${header_overhead}" -d "${prop_delay}" -n "${numflowsfinish}" -m "${numflowsstart}" -t "${maxtimeslots}" -q "${numdatapoints}"

