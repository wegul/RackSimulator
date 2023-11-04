# List all files
for file in *shuffle.csv; do
    if [ -f "$file" ]; then
        echo "python3 ./scripts/reverse_mem.py -fi $file -fo uni_$file"
        python3 ./scripts/reverse_mem.py -fi $file -fo uni_$file
    fi
done


# python3 ./scripts/reverse_mem.py -fi mn1-9.csv -fo test00.csv