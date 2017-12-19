#!/bin/bash

function create_link_file()
{
	real_file=$3/*$4
	all_files=`ls -lf $real_file`
	path_len=`expr length $3`
	start_pos=`expr $path_len + 2`

	for file in $all_files
	do
		pos=`expr index $file .`
		len=`expr $pos - $start_pos`
		pre=`expr substr $file $start_pos $len`
		rm -vf $1/$pre$2 >> /dev/null 2>&1
		ln -s $file $1/$pre$2
	done
}


HLR_DIR=`pwd`
CHARGE_LIB_PATH=${HLR_DIR}/linux/charge_agt_lib
MEMDB_LIB_PATH=${HLR_DIR}/linux/mem_db_lib

create_link_file $CHARGE_LIB_PATH .so.1 $CHARGE_LIB_PATH .so.1.0
create_link_file $CHARGE_LIB_PATH .so $CHARGE_LIB_PATH .so.1
create_link_file $CHARGE_LIB_PATH .so $CHARGE_LIB_PATH .so.5.4.10
create_link_file $CHARGE_LIB_PATH .so $CHARGE_LIB_PATH .so.1.4.10
create_link_file $MEMDB_LIB_PATH .so $MEMDB_LIB_PATH .so.2
