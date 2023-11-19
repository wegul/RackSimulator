cd ./

if [[ $# -eq 0 ]]; then
    echo "Please provide a directory path."
    exit 1
fi

# Get the directory path from the first argument
name=$1

# List all files
for file in *${name}.csv; do
    if [ -f "$file" ]; then
        echo "~/RackSimulator/simulation/bin/driver -f $file -b 100 > log${1}.txt"
        ~/RackSimulator/simulation/bin/driver -f $file -b 100 >log${1}.txt
    fi
done
