cd ./

SCRIPT_DIR="$HOME/Desktop/RackSimulator/scripts"


for file in *rrreq*.csv; do
    if [[ -f $file ]]; then
        prefix=${file%rrreq*}
        postfix=${file#*rrreq}
        echo "mv ${file} ${prefix}rreq${postfix}"
        mv ${file} ${prefix}rreq${postfix}
        # mv ${file} ${prefix}wreq${postfix}
    fi
done

# for file in proced_8B*wreq*.csv; do
#     if [[ -f $file ]]; then
#         prefix=${file%wreq*}
#         postfix=${file#*wreq}
#         echo "python3 ${SCRIPT_DIR}/reverse_mem.py -fi ${file} -fo ${prefix}rresp${postfix}"
#         python3 ${SCRIPT_DIR}/reverse_mem.py -fi ${file} -fo ${prefix}rresp${postfix}
#     fi
# done