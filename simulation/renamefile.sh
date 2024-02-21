cd ./

mkdir -p ./rreq
mkdir -p ./wreq
for file in *.out; do
    if [[ -f ${file} ]]; then
        if [[ "${file}" == rreq* ]]; then
            echo "mv ${file} ./rreq/${file%.out}"
            mv ${file} ./rreq/${file%.out}
        else
            echo "mv ${file} ./wreq/${file%.out}"
            mv ${file} ./wreq/${file%.out}
        fi
    fi
done
