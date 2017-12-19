#!/bin/bash

if [ "$1" == "clean" ]
then
  find . -type f -name *.o |xargs rm -rf 2 >/dev/null
  find . -type f -name *.d |xargs rm -rf 2 >/dev/null
  echo "clean *.o *.d complete!"
  exit
fi

# ÇëÐÞ¸ÄsetenvÖÐµÄSRC_ROOT
. ./setenv

# build for unit test
if [ "$1" == "ut" ]
then
  unset XOS_NEED_MAIN
fi

echo ------------- make xos  -------------
make

echo ------------- make port -------------
cd ./port
make -f xos_op.mak

echo ------------- cp xos_op.a -------------
cd ..

if [ ! -e lib ]
then
  mkdir lib
fi

if [ "$1" == "ut" ]
then
  cp ./obj/linux/cpsv3000/port/xos_op.a ./lib/libxos_op_unit_test.a
#  cp ./obj/linux/cpsv3000/port/xos_op.a ../../../../lib/libxos_op_unit_test.a
else
  cp ./obj/linux/cpsv3000/port/xos_op.a ./lib/libxos_op.a
#  cp ./obj/linux/cpsv3000/port/xos_op.a ../../../../lib/libxos_op.a
fi
