cd ./
for file in proced*.csv; do
    if [ -f "$file" ]; then
        echo "~/Desktop/RackSimulator/simulation/bin/driver -f $file -n 128  > log-${file}-128.txt"
        ~/Desktop/RackSimulator/simulation/bin/driver -f $file -n 128 >log-${file}-128.txt
    fi
done
