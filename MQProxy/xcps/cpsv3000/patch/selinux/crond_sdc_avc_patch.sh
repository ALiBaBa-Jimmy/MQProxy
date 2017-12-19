#!/bin/bash
#auth: maliming.xinwei
#date:2014-7-1

#增加selinux的AVC策略

#模块名称
export CROND_SDC_MODULE_NAME="crondsdc"
#版本号
export CROND_SDC_VERSION="1.0.1"
#访问策略
export CROND_SDC_POLICY="create getopt setopt bind connect accept ioctl read write"
#主体--进程
export CROND_SDC_SRC_TYPE="system_cronjob_t"
#客体--被进程访问的对象
export CROND_SDC_DEST_TYPE="unlabeled_t"

export CROND_SDC_TE=./$CROND_SDC_MODULE_NAME.te

rm -rf $CROND_SDC_TE $CROND_SDC_MODULE_NAME.mod $CROND_SDC_MODULE_NAME.pp

#生成TE文件
echo -e "module $CROND_SDC_MODULE_NAME $CROND_SDC_VERSION;\n\n" >> $CROND_SDC_TE

echo -e "require{" >> $CROND_SDC_TE
echo -e	"    type $CROND_SDC_SRC_TYPE;" >> $CROND_SDC_TE
echo -e "    type $CROND_SDC_DEST_TYPE;" >> $CROND_SDC_TE
echo -e	"    class rawip_socket { $CROND_SDC_POLICY };" >> $CROND_SDC_TE
echo -e	"    class capability net_raw;" >> $CROND_SDC_TE
echo -e "}\n\n" >> $CROND_SDC_TE

echo -e "#===================$CROND_SDC_SRC_TYPE================" >> $CROND_SDC_TE
echo -e "#!!!!This avc is allowed in the current policy" >> $CROND_SDC_TE
echo -e "allow $CROND_SDC_SRC_TYPE $CROND_SDC_DEST_TYPE:capability net_raw;" >> $CROND_SDC_TE
echo -e "allow $CROND_SDC_SRC_TYPE $CROND_SDC_DEST_TYPE:rawip_socket { $CROND_SDC_POLICY };" >> $CROND_SDC_TE

#编译策略
checkmodule -M -m -o $CROND_SDC_MODULE_NAME.mod $CROND_SDC_MODULE_NAME.te

semodule_package -o $CROND_SDC_MODULE_NAME.pp -m $CROND_SDC_MODULE_NAME.mod

#重新加载策略
CROND_SDC_GREP=`semodule -l|grep $CROND_SDC_MODULE_NAME`
if [ "${CROND_SDC_GREP}" != "" ]
then
    echo "checkmodule:  remove module ${CROND_SDC_GREP}"
    semodule -r crondsdc.pp
fi

echo "checkmodule:  insert module $CROND_SDC_MODULE_NAME    $CROND_SDC_VERSION"
semodule -u crondsdc.pp

#删除临时文件
rm -rf $CROND_SDC_TE $CROND_SDC_MODULE_NAME.mod $CROND_SDC_MODULE_NAME.pp

echo -e "checkmodule:  ^_^ @_@ ^_^ \ncheckmodule:  crond $CROND_SDC_SRC_TYPE:$CROND_SDC_DEST_TYPE for sdc selinux avc insert success!"
