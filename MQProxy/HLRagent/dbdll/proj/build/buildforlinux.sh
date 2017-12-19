#!/bin/bash

export PRG_ROOT=`pwd`/../..
export PARENT_PRG_ROOT=$PRG_ROOT/..

export COMM_BUILD_ENV=$PARENT_PRG_ROOT/proj/build/comm_build_env

chmod +x  ${COMM_BUILD_ENV}
dos2unix ${COMM_BUILD_ENV}
source ${COMM_BUILD_ENV}


cd $BIN_DIR
chmod +x  $BIN_DIR/*.sh
dos2unix $BIN_DIR/*.sh

cd $BUILD_DIR

chmod +x  $BUILD_DIR/build_dbproj
dos2unix $BUILD_DIR/build_dbproj
$BUILD_DIR/build_dbproj $2 2> errorbuild_dbproj



echo ============ Copy to publish path =============
PUBLISH_DLL_PATH=$PUBLISTH_ROOT/dbdll
PUBLISH_DLL_NAME=libdbproj.so*
rm -rf $PUBLISH_DLL_PATH/
mkdir $PUBLISH_DLL_PATH

echo PRG_INCLUDE_PATH=$PRG_INCLUDE_PATH
echo PUBLISH_DLL_PATH=$PUBLISH_DLL_PATH
cp -rf $PRG_INCLUDE_PATH/ $PUBLISH_DLL_PATH

echo BIN_DIR=$BIN_DIR
echo PUBLISH_DLL_PATH=$PUBLISH_DLL_PATH
cp -rf $BIN_DIR/ $PUBLISH_DLL_PATH/

echo BIN_LINUX_DIR=$BIN_LINUX_DIR
echo PUBLISTH_ROOT=$PUBLISTH_ROOT
cp -rf $BIN_LINUX_DIR/lib/$PUBLISH_DLL_NAME $PUBLISTH_ROOT/bin/linux
cp -rf $BIN_LINUX_DIR/lib/$PUBLISH_DLL_NAME $PUBLISTH_ROOT/bin/linux/lib
cp -rf $BIN_LINUX_DIR/lib/$PUBLISH_DLL_NAME $PUBLISTH_ROOT/dbdll/bin/linux/lib/
cp -rf $ORACLE_LIB_ROOT/lib/*  $PUBLISTH_ROOT/bin/linux/lib
cp -rf $ACE_LIB_ROOT/lib/*  $PUBLISTH_ROOT/bin/linux/lib


echo ============ 处理lib，为方便以后加版本号,启用模块引用lib目录为$PUBLISTH_ROOT/dbdll/bin/linux =============
PUBLISH_DLL_LINUX=$PUBLISH_DLL_PATH/bin/linux
echo PUBLISH_DLL_LINUX = $PUBLISH_DLL_LINUX
cd $PUBLISH_DLL_LINUX
chmod +x  ${PUBLISH_DLL_LINUX}/*.sh
dos2unix ${PUBLISH_DLL_LINUX}/*.sh
${PUBLISH_DLL_LINUX}/link.sh
