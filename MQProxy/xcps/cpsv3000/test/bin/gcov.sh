#! /bin/bash

if [ "$XCPS_ROOT" == "" ] ;then
  XCPS_ROOT=../../..
fi

chmod +x $XCPS_ROOT/tools/lcov/*
LCOV=$XCPS_ROOT/tools/lcov/lcov
GENHTML=$XCPS_ROOT/tools/lcov/genhtml

if [ "$1" == "clean" ] ;then
	rm -f ./*.info
	rm -f ./gmon.out
	rm -rf ./gcov
	$LCOV --zerocounters --directory $XCPS_ROOT
else
	XCPS_PATH=$(cd ../../../; pwd)  ###相对路径转绝对路径
	$LCOV  --capture --directory $XCPS_PATH  --output-file test.info --test-name test_noargs --no-external
	echo -e "---------------------------------lcov end! genhtml begin--------------------------------\n"
	$GENHTML test.info  --output-directory ./gcov  --title "XOS TEST" --show-details --legend
fi
