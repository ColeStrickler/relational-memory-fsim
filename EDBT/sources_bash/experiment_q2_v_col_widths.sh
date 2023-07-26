#!/bin/bash

ODIR=../objects

NUM_REVS=4
NUM_SAMPLES=30 #$1

ROW_SIZE=64
ROW_COUNT=43690
ENABLED_COL_NUM=2

COL_OFF="0,32"

COL_WIDTHS="1 2 4 8 16"

FRAME_OFF=0x0

K=0x88

BENCH=q2_col
VERSIONS="v4_multi_col"

for version in ${VERSIONS}
do
    mv PLT_result_${BENCH}.csv PLT_result_${BENCH}.csv_old 2>/dev/null
    touch PLT_result_${BENCH}.csv
    echo "bench, mem, temp, row_size, row_count, col_width, cycles" >> PLT_result_${BENCH}.csv
    for col_width in ${COL_WIDTHS}
    do

        for ((sample = 1 ; sample <= $((NUM_SAMPLES)) ; sample++)) 
        do
       	    #bash update_relcache_version.sh $version #> /dev/null &
       	    wait $!        	

            #populate
            num_columns=$((${ROW_SIZE}/${col_width}))
			width="${col_width}"
			for ((num= 2 ; num<= $((num_columns)) ; num++)) 
			do
				width="${width},${col_width}"
			done
       	    ${ODIR}/db_generate -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${num_columns} -W ${width} #> /dev/null
			echo "populate done."

       	    #config
			width="${col_width}"
			for (( num= 1 ; num< $((ENABLED_COL_NUM)) ; num++ )) 
			do
				width="${width},${col_width}"
			done
       	    ${ODIR}/db_config_relcache -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} #> /dev/null
			echo "config done."

       	    #execution query
       	    ${ODIR}/${BENCH}${col_width} -r ${ROW_SIZE} -R ${ROW_COUNT} -C ${ENABLED_COL_NUM} -W ${width} -O ${COL_OFF} -F ${FRAME_OFF} ${K} >> PLT_result_${BENCH}.csv &
       	    wait $!
			echo "query done"

            ${ODIR}/db_reset_relcache 0
            ${ODIR}/db_reset_relcache 1
            wait $!    		
        done
    done
done

