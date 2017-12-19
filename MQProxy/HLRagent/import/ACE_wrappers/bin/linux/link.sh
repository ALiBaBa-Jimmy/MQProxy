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
rm -rf libA* liba* libG*  libH*  libJ* libK* libn*  libw*  
cp -rf $LIB_PATH/* $LIB_TARGE_PATH/
ln -s libACE.so.5.7.0 libACE.so
ln -s libACE_SSL.so.5.7.0 libACE_SSL.so
ln -s libACEXML_Parser.so.5.7.0 libACEXML_Parser.so
ln -s libACEXML.so.5.7.0 libACEXML.so
  
