current_micro_dir="$(pwd)"
echo "$current_micro_dir"

for dir in *; do
    if [[ -d "$dir" ]]; then
        echo "cd $dir"
        cd $dir
        outpath="./out"
        para_n="${dir}"
        para_n=${para_n:2}
        echo "Current running -n $para_n"

        if [[ ! -d "$outpath" ]]; then
            echo "Directory created: mkdir -p $outpath"
            mkdir -p "$outpath"
        fi

        for file in proced*.csv; do
            if [ -f "$file" ]; then
                echo "~/Desktop/RackSimulator/simulation/bin/driver -f $file -n ${para_n}  > log-${file}-${para_n}.txt"
                ~/Desktop/RackSimulator/simulation/bin/driver -f $file -n ${para_n} >log-${file}-${para_n}.txt
            fi
        done
        echo "cd $current_micro_dir"
        cd $current_micro_dir
    fi

done
