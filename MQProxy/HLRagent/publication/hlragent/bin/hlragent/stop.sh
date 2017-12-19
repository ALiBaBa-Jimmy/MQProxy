#!/bin/bash

path=`dirname $0`
path=$(cd $path; pwd)
EXE=hlragent.out

################ don't modify the code below #################
TCN_NE_ID=$1
TCN_WORKSPACE_ID=$2
TCN_PROC_ID=$3
NEW_EXE_NAME=${EXE}_

XOS_PID=`ps -ef |grep $NEW_EXE_NAME|grep $path | awk '{print $2}'`;

cd $path
rm -f $NEW_EXE_NAME

if test -z ${XOS_PID}; then
    echo ${path}/${NEW_EXE_NAME} not start
    exit
fi
[ "" != "${XOS_PID}" ] && kill -9 ${XOS_PID}
############################################################
