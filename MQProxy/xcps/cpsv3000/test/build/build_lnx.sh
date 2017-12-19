#!/bin/bash

if [ $# != 1 ]; then
	echo -e "\nArgument error!"
	echo -e "Plese input: $0 [ exe / clean ]\n"
	exit 1
fi

. ./setenv

if [ "$1" == "exe" ]; then
	echo ------------- make xostest -------------
	mkdir -p $OBJS_DIR
	make -C $OBJS_DIR -f $XOSTEST_ROOT/build/xos_test.mk
	rm -rf ../obj/linux/cpsv3000/test/build   ###删除无用目录
elif [ "$1" == "clean" ]; then
	if [ "$XOS_ENABLE_GCOV" == "1" ] && [ -f $OBJS_DIR/debug/gcov.sh ] ;then
		$OBJS_DIR/debug/gcov.sh clean
	fi
	rm -rf $OBJS_DIR
else
	echo -e "Plese input: $0 [ exe / clean ]\n"
	exit 1
fi
 