current_overall_dir="$(pwd)"
echo "$current_overall_dir"

outpath="./out"

for file in *.csv; do #run
    if [ -f "$file" ]; then

        if [[ "${file}" == "mix"*"graphlab0.4"* ]]; then
            echo "~/Desktop/RackSimulator/simulation/bin/driver -f $file -n 256 > log-${file}.txt"
            ~/Desktop/RackSimulator/simulation/bin/driver -f $file -n 256 else >log-${file}.txt
        else
            if [[ "${file}" == "mix"*"graphlab0.6"* ]]; then
                echo "~/Desktop/RackSimulator/simulation/bin/driver -f $file -n 256 > log-${file}.txt"
                ~/Desktop/RackSimulator/simulation/bin/driver -f $file -n 256 >log-${file}.txt
            fi
        fi
    fi
done
