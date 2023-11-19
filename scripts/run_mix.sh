cd ./

for file in mix*.csv; do
    if [ -f "$file" ]; then
        echo "~/Desktop/RackSimulator/simulation/bin/driver -f $file -b 100 > log_mix.txt"
        ~/Desktop/RackSimulator/simulation/bin/driver -f $file -b 100 >log_mix.txt
    fi
done
