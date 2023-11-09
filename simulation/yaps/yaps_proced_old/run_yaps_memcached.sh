cd ./

# List all files
for file in proced\_mem*.csv; do
    if [ -f "$file" ]; then
        echo "~/RackSimulator/simulation/bin/driver -f $file -b 100 > log_mem.txt"
        ~/RackSimulator/simulation/bin/driver -f $file -b 100 >log_mem.txt
    fi
done
