cd ./
for file in proced*.csv; do
    if [ -f "$file" ]; then
        echo "~/Desktop/RackSimulator/simulation/bin/driver -f $file -n 64  > log-${file}-64.txt"
        ~/Desktop/RackSimulator/simulation/bin/driver -f $file -n 64 >log-${file}-64.txt
    fi
done
