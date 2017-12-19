#!/bin/bash

path=`dirname $0`
path=$(cd $path; pwd)
EXE=hlragent.out

################ don't modify the code below #################
export TCN_NE_ID=$1
export TCN_PROC_ID=$3

export XOS_PATH_ENV=$path/$EXE
chmod u+x $XOS_PATH_ENV

NEW_EXE_NAME=${path}/${EXE}_

rm -f $NEW_EXE_NAME
cd $path
ln -s ./$EXE $NEW_EXE_NAME 


ulimit -c unlimited
sysctl -w kernel.core_pattern=core.%p.%e.%t > /dev/null
##############################################################

######################### start process ######################
export ORACLE_HOME=/opt/oracle/product/ora11g
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}:${path}/lib:/opt/oracle/product/ora11g/lib
$NEW_EXE_NAME -D
