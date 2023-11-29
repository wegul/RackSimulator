SCRPT="${HOME}/Desktop/RackSimulator/scripts/IdealSimulator.py"
cd ./

for file in *.csv; do
    echo python3 ${SCRPT} -fi $file -fo ideal-${file}
    python3 ${SCRPT} -fi $file -fo ideal-${file}
    # python3 ${SCRPT} -fi $file -fo out/ideal-${file}
done
