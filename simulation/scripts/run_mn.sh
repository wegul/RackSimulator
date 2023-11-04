cd ./

for file in *mem_shuffled.csv; do
    if [ -f "$file" ]; then
        bw=$((${file:2:1}*10))
        echo "~/RackSimulator/simulation/bin/driver -f $file -b $bw"
        ~/RackSimulator/simulation/bin/driver -f $file -b $bw
    fi
done

for file in *net_shuffled.csv; do
    if [ -f "$file" ]; then
        bw=$((${file:4:1}*10))
        echo "~/RackSimulator/simulation/bin/driver -f $file -b $bw"
        ~/RackSimulator/simulation/bin/driver -f $file -b $bw
    fi
done

# List all files
for file in *shuffle.csv; do
    if [ -f "$file" ]; then
        echo "~/RackSimulator/simulation/bin/driver -f $file -b 100"
        ~/RackSimulator/simulation/bin/driver -f $file -b 100
    fi
done
