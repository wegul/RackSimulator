#!/bin/bash

# Rename files
current_micro_dir="$(pwd)"
echo "$current_micro_dir"

outdir="chunksize-out"
if [[ ! -d outdir ]]; then
    echo "Directory created: mkdir -p ${outdir}"
    mkdir -p "${outdir}"
fi

for dir in *; do
    if [[ -d "$dir" && "${dir}" != ${outdir} ]]; then
        echo "cd ${dir}/out"
        cd ${dir}/out
        para_n="${dir}"
        para_n=${para_n:2}

        for file in proced*.out; do
            if [ -f "$file" ]; then
                newname="${file%.out}"
                newname="n${para_n}-${newname:7}"
                echo "cp ${file} ${current_micro_dir}/${outdir}/${newname}"
                cp ${file} ${current_micro_dir}/${outdir}/${newname}
            fi
        done
        echo "cd $current_micro_dir"
        cd $current_micro_dir
    fi
done
