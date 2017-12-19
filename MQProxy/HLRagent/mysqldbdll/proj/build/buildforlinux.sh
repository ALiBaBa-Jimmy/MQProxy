#!/bin/bash

if [ ! -n "$1" ] ; then
	echo "Usage: buildforlinux.sh errorExportToScreen[0:not export; 1:export]"
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


echo ==========BUILD ${BUILD_MODULE_NAME}==============================
cd $BUILD_DIR

chmod +x  $BUILD_DIR/build_lnx
dos2unix $BUILD_DIR/build_lnx

if [ "$1" = "0" ];then	
  $BUILD_DIR/build_lnx $2 2> errorbuild_${BUILD_MODULE_NAME}
else
  $BUILD_DIR/build_lnx $2 
fi

LIB_BUILD_SUCCESS=`find $PRG_ROOT/bin/linux/lib/*`
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
PUBLISH_DLL_PATH=$PUBLISTH_ROOT/${BUILD_MODULE_NAME}
PUBLISH_DLL_NAME=lib${BUILD_MODULE_NAME}.so*
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
cp -rf $BIN_LINUX_DIR/lib/$PUBLISH_DLL_NAME $PUBLISTH_ROOT/bin/linux/lib
cp -rf $MYSQL_LIB_ROOT/lib* $PUBLISTH_ROOT/bin/linux/lib
cp -rf $ACE_LIB_ROOT/lib/* $PUBLISTH_ROOT/bin/linux/lib



echo ============ 处理lib，为方便以后加版本号,启用模块引用lib目录为$PUBLISTH_ROOT/${BUILD_MODULE_NAME}/bin/linux =============
PUBLISH_DLL_LINUX=$PUBLISH_DLL_PATH/bin/linux
echo PUBLISH_DLL_LINUX = $PUBLISH_DLL_LINUX
cd $PUBLISH_DLL_LINUX
chmod +x  ${PUBLISH_DLL_LINUX}/*.sh
dos2unix ${PUBLISH_DLL_LINUX}/*.sh
${PUBLISH_DLL_LINUX}/link.sh



#============ 能正常运行测试用例 =============
echo BIN_LINUX_DIR = $BIN_LINUX_DIR
echo PUBLISTH_ROOT/bin/linux/ = $PUBLISTH_ROOT/bin/linux/
cp -rf $BIN_LINUX_DIR/* $PUBLISTH_ROOT/bin/linux/
cd $PUBLISTH_ROOT/bin/linux
chmod +x  $PUBLISTH_ROOT/bin/linux/*.sh
dos2unix $PUBLISTH_ROOT/bin/linux/*.sh
$PUBLISTH_ROOT/bin/linux/link.sh


