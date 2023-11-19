#!/bin/bash

# Copy files to all-traces

# Check if a name is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <name>"
    exit 1
fi

outname=$1
mkdir -p $outname

files13=("mix_"*"0.1"*"0.3"*".csv")
for file in "${files13[@]}"; do
    echo cp ${file} ${outname}/${file}
    cp ${file} ${outname}/${file}
done

files22=("mix_"*"0.2"*"0.2"*".csv")
for file in "${files22[@]}"; do
    echo cp ${file} ${outname}/${file}
    cp ${file} ${outname}/${file}
done

files31=("mix_"*"0.3"*"0.1"*".csv")
for file in "${files31[@]}"; do
    echo cp ${file} ${outname}/${file}
    cp ${file} ${outname}/${file}
done

files26=("mix_"*"0.2"*"0.6"*".csv")
for file in "${files26[@]}"; do
    echo cp ${file} ${outname}/${file}
    cp ${file} ${outname}/${file}
done

files44=("mix_"*"0.4"*"0.4"*".csv")
for file in "${files44[@]}"; do
    echo cp ${file} ${outname}/${file}
    cp ${file} ${outname}/${file}
done

files62=("mix_"*"0.6"*"0.2"*".csv")
for file in "${files62[@]}"; do
    echo cp ${file} ${outname}/${file}
    cp ${file} ${outname}/${file}
done
