#!/bin/bash


col_width=$(awk -F'=' '/column_width/ {print $2}' config | tr -d ' ')
sed -i "s/column_width = $col_width/column_width = $1/" config

store_type=$(awk -F'=' '/store_type/ {print $2}' config | tr -d ' ')
sed -i "s/store_type = $store_type/store_type = $2/" config
