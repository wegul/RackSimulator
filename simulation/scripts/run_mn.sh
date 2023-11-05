cd ./

for file in *mem.csv; do
    if [ -f "$file" ]; then
        bw=$((${file:2:1} * 10))
        echo "~/RackSimulator/simulation/bin/driver -f $file -b $bw >log.txt"
        ~/RackSimulator/simulation/bin/driver -f $file -b $bw >log.txt
    fi
done

for file in *net.csv; do
    if [ -f "$file" ]; then
        bw=$((${file:4:1} * 10))
        echo "~/RackSimulator/simulation/bin/driver -f $file -b $bw >log.txt"
        ~/RackSimulator/simulation/bin/driver -f $file -b $bw >log.txt
    fi
done

# Run hybrid for 100G
for file in *full.csv; do
    if [ -f "$file" ]; then
        echo "~/RackSimulator/simulation/bin/driver -f $file -b 100 >log.txt"
        ~/RackSimulator/simulation/bin/driver -f $file -b 100 >log.txt
    fi
done
