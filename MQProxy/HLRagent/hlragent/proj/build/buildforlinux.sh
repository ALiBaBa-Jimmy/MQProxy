#!/bin/bash

if [ ! -n "$1" ] ; then
	echo "Usage: buildforlinux.sh errorExportToScreen[0:not export; 1:export] name"
	exit 0
fi

if [ ! -n "$2" ] ; then
	echo "Usage: buildforlinux.sh errorExportToScreen[0:not export; 1:export] name"
	exit 0
fi

export PRG_ROOT=`pwd`/../..
export PARENT_PRG_ROOT=$PRG_ROOT/..

echo TOTAL_BUILD_MODULE_ROOT=$TOTAL_BUILD_MODULE_ROOT
export COMM_BUILD_ENV=$PARENT_PRG_ROOT/proj/build/comm_build_env
echo BUILD_MODULE_NAME=$BUILD_MODULE_NAME
echo "*******build_forlinux BUILD_MODULE_NAME=${BUILD_MODULE_NAME}********"

chmod +x  ${COMM_BUILD_ENV}
dos2unix ${COMM_BUILD_ENV}
source ${COMM_BUILD_ENV}

cd $BIN_DIR
chmod +x  $BIN_DIR/*.sh
dos2unix $BIN_DIR/*.sh



echo ==========BUILD ${BUILD_MODULE_NAME}==============================
cd $BUILD_DIR

chmod +x  $BUILD_DIR/build_lnx
dos2unix $BUILD_DIR/build_lnx

if [ "$1" = "0" ];then	
  $BUILD_DIR/build_lnx $2 2> errorbuild_${BUILD_MODULE_NAME}
else
  $BUILD_DIR/build_lnx $2 
fi

LIB_BUILD_SUCCESS=`find $PRG_ROOT/bin/linux/hlragent.out`
echo LIB_BUILD_SUCCESS=$LIB_BUILD_SUCCESS

if [ "" != "$LIB_BUILD_SUCCESS" ]; then
	echo =================Build $PRG_ROOT SUCCESED=======================
	rm -f ${PRG_ROOT}/../proj/build/errorbuild_${BUILD_MODULE_NAME}
else
	echo =================Build $PRG_ROOT FAILED=======================
	cp -f ${BUILD_DIR}/errorbuild_${BUILD_MODULE_NAME} ${PRG_ROOT}/../proj/build
	exit 2
fi

echo ============ Copy to publish path =============
PUBLISH_EXE_PATH=$PUBLISTH_ROOT/${BUILD_MODULE_NAME}
PUBLISH_EXE_NAME=${BUILD_MODULE_NAME}
rm -rf $PUBLISH_EXE_PATH/
mkdir $PUBLISH_EXE_PATH
echo PRG_INCLUDE_PATH=$PRG_INCLUDE_PATH
echo PUBLISH_EXE_PATH=$PUBLISH_EXE_PATH
cp -rf $PRG_INCLUDE_PATH/ $PUBLISH_EXE_PATH
echo BIN_DIR=$BIN_DIR
echo PUBLISH_EXE_PATH=$PUBLISH_EXE_PATH
cp -rf $BIN_DIR/ $PUBLISH_EXE_PATH/
echo BIN_LINUX_DIR=$BIN_LINUX_DIR
echo PUBLISTH_ROOT=$PUBLISTH_ROOT
cp -rf $BIN_LINUX_DIR/$PUBLISH_EXE_NAME $PUBLISTH_ROOT/bin/linux/



#echo ============ 处理exe，为方便以后加版本号,启用模块引用lib目录为$PUBLISTH_ROOT/${BUILD_MODULE_NAME}/bin/linux =============
#PUBLISH_DLL_LINUX=$PUBLISH_DLL_PATH/bin/linux
#echo PUBLISH_DLL_LINUX = $PUBLISH_DLL_LINUX
#cd $PUBLISH_DLL_LINUX
#chmod +x  ${PUBLISH_DLL_LINUX}/*.sh
#dos2unix ${PUBLISH_DLL_LINUX}/*.sh
#${PUBLISH_DLL_LINUX}/link.sh


#echo ============ copy all lib  =============
echo PUBLISTH_ROOT=$PUBLISTH_ROOT
echo BIN_DIR=$BIN_DIR
cp -rf  $PUBLISTH_ROOT/bin/linux/lib/* $BIN_DIR/linux/lib

cd $BIN_DIR
$BIN_DIR/packet.sh $2

cp -rf  $BIN_DIR/$2.tar.gz $PWD_PRG_ROOT
