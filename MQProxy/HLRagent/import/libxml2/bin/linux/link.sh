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


PROG_LINUX_DIR=`pwd`
LIB_PATH=${PROG_LINUX_DIR}/lib
LIB_TARGE_PATH=${PROG_LINUX_DIR}

echo LIB_PATH=$LIB_PATH
cp -rf $LIB_PATH/* $LIB_TARGE_PATH/
create_link_file $LIB_TARGE_PATH .so $LIB_TARGE_PATH .so.*

