current_overall_dir="$(pwd)"
echo "$current_overall_dir"

outpath="./out"

# if [[ ! -d "$outpath" ]]; then
#     echo "Directory created: mkdir -p $outpath"
#     mkdir -p "$outpath"
# fi

for file in proced*.csv; do #convert
    if [ -f "$file" ]; then
        if [[ ${file} == proced*csv || ${file} == mix*csv ]]; then #Not-yet converted
            num="${file#*-100G}"
            echo "$num"
            firstname="${file%-trace*}"
            firstname=${firstname#proced_}
            echo "netVer_${firstname}${num}"
            newname="../netVer_${firstname}${num}"
            echo "python3 ~/Desktop/RackSimulator/scripts/convert_proced_to_net.py -fi $file -fo $newname"
            python3 ~/Desktop/RackSimulator/scripts/convert_proced_to_net.py -fi $file -fo $newname
        fi
    fi
done

for file in *.csv; do #run
    if [ -f "$file" ]; then
        if [[ "${file}" != "proced"* ]]; then
            echo "~/Desktop/RackSimulator/simulation/bin/driver -f $file > log-${file}.txt"
            ~/Desktop/RackSimulator/simulation/bin/driver -f $file >log-${file}.txt
        fi
    fi
done
