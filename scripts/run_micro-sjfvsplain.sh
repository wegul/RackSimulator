current_micro_dir="$(pwd)"
echo "$current_micro_dir"

for dir in *; do
    if [[ -d "$dir" ]]; then
        echo "cd $dir"
        cd $dir
        outpath="./out"
        para_n="${dir}"
        para_n=$(echo "$para_n" | grep -oE '[0-9]+')
        echo "Current running -n $para_n"

        if [[ ! -d "$outpath" ]]; then
            echo "Directory created: mkdir -p $outpath"
            mkdir -p "$outpath"
        fi

        for file in proced_tera*.csv; do
            if [ -f "$file" ]; then
                if [[ ${dir} == plain* ]]; then
                    echo "~/Desktop/RackSimulator/simulation/bin/driver-chunk-plain -f $file -n ${para_n}  > log-${file}-${para_n}.txt"
                    ~/Desktop/RackSimulator/simulation/bin/driver-chunk-plain -f $file -n ${para_n} >log-${file}-${para_n}.txt
                fi
            fi
        done
        echo "cd $current_micro_dir"
        cd $current_micro_dir
    fi
done
