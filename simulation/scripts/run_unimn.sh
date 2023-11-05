cd ./

# List all files
for file in uni_*; do
    if [ -f "$file" ]; then
        echo "~/RackSimulator/simulation/bin/driver -f $file -b 100 > log.txt"
        ~/RackSimulator/simulation/bin/driver -f $file -b 100 >log.txt
    fi
done
