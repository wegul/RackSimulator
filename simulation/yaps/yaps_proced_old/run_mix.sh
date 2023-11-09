cd ./

for file in mix*.csv; do
    if [ -f "$file" ]; then
        # echo python3 ~/RackSimulator/simulation/scripts/shuffle.py -fi "${file}" -fo "${file}"
        # python3 ~/RackSimulator/simulation/scripts/shuffle.py -fi "${file}" -fo "${file}"
        echo "~/Desktop/RackSimulator/simulation/bin/driver -f $file -b 100 > log_mix.txt"
        ~/Desktop/RackSimulator/simulation/bin/driver -f $file -b 100 >log_mix.txt
    fi
done
