current_overall_dir="$(pwd)"
echo "$current_overall_dir"

outpath="./out"

for file in proced*.csv; do #run
    if [ -f "$file" ]; then
        if [[ "${file}" == "proced"* ]]; then
            echo "~/Desktop/RackSimulator/simulation/bin/driver-strictmatching -f $file -n 256 > log-${file}.txt"
            ~/Desktop/RackSimulator/simulation/bin/driver-strictmatching -f $file -n 256 >log-${file}.txt
        fi

    fi
done
