cd ./

program="../simulation/bin/driver-dctcp"

for file in *.csv; do
    if [[ -f ${file} ]]; then
        echo "screen -dm bash -c \"$program -f $file >log${file}.txt\""
        screen -dm bash -c "${program} -f ${file} >log${file}.txt"
    fi
done
