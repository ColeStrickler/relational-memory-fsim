#!/bin/bash


col_width=$(awk -F'=' '/column_width/ {print $2}' config | tr -d ' ')
sed -i "s/column_width = $col_width/column_width = $1/" config

store_type=$(awk -F'=' '/store_type/ {print $2}' config | tr -d ' ')
sed -i "s/store_type = $store_type/store_type = $2/" config

rcol=$(awk -F'=' '/r_col/ {print $2}' config | tr -d ' ')
sed -i "s/r_col = $rcol/r_col = $3/" config

num_col=$(awk -F'=' '/num_columns/ {print $2}' config | tr -d ' ')
sed -i "s/num_columns = $num_col/num_columns = $4/" config

