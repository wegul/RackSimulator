cd ./

# List all files
for file in *shuffle.csv; do
    if [ -f "$file" ]; then
        echo "~/RackSimulator/simulation/bin/driver -f $file -b 100"
        ~/RackSimulator/simulation/bin/driver -f $file -b 100
    fi
done
