SCRIPT_DIR="$HOME/Desktop/RackSimulator/simulation/scripts"

if [[ "$#" < 1 ]]; then
    echo "Usage: $0 <output path name> <bandwidth>"
    exit 1
fi

prefix=$1
cnt=0

for file in ./${prefix}*.csv; do
    if [ -f "$file" ]; then
        cnt+=1
        echo "~/Desktop/RackSimulator/simulation/bin/driver -f $file -b 100 > log_${prefix}_${cnt}.txt"
        ~/Desktop/RackSimulator/simulation/bin/driver -f $file -b 100 >log_${prefix}_${cnt}.txt
    fi
done
