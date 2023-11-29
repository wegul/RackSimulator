SCRPT="${HOME}/Desktop/RackSimulator/scripts/IdealSimulatorRREQ.py"
cd ./

for dir in *; do
    if [[ -d $dir ]]; then
        cd $dir
        mkdir out
        for file in *.csv; do
            echo python3 ${SCRPT} -fi $file -fo out/ideal-${file}
            python3 ${SCRPT} -fi $file -fo out/ideal-${file}
        done
        cd ../
    fi
done
