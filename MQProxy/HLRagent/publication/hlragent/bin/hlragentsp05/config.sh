#!/bin/sh
#..............................................................................
# HLRagent Parameter Configuration
#
# author: 	wdf
# date:		2015-9-10
# ver: 		1.0
#.............................................................................

product_path=""

function HLR_get_product_path()
{
	
	filename=$1
	parameter=$2
	product_path=`cat $filename | grep $parameter | awk -F '[=]' '{print $2}'` 
}
function HLR_delete_line_match()
{
	
	filename=$1
	matchvalue=$2

	sed -i '/'"$matchvalue"'/d' $filename
}
function HLR_replace_product_path()
{
	
	filename=$1
	oldvalue=$2
	newvalue=$3
	sed -i 's/^*='"$oldvalue"'/^*='"$newvalue"'/g' $filename
}
function HLR_get_parameter_value()
{
	
	filename=$1
	parameter=$2
	parameter_value=`cat $filename | grep $parameter | awk -F '[<>]' '{print $3}'` 
}
function HLR_replace_parameter_value()
{
	
	filename=$1
	oldvalue=$3
	newvalue=$4
	sed -i 's/<'"$2"'>'"$oldvalue"'/<'"$2"'>'"$newvalue"'/g' $filename
}

tput clear
echo "###################################################"
echo "**********HLRagent Parameter Configuration*********"
echo "###################################################"

if [ ! -f /etc/HLRagent_profile ]; then

	touch /etc/HLRagent_profile
	chmod u+x /etc/HLRagent_profile
fi

HLR_get_product_path /etc/HLRagent_profile HLRAGENT_PATH=

if [  "" = "$product_path" ]; then
	echo 'export EXE=hlragent.out'>>/etc/HLRagent_profile 
	echo export HLRAGENT_PATH="$PWD">>/etc/HLRagent_profile
	echo export ORACLE_HOME=/opt/oracle/product/ora11g>>/etc/HLRagent_profile
	echo 'export LD_LIBRARY_PATH=${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}:${HLRAGENT_PATH}/lib:${ORACLE_HOME}/lib'>>/etc/HLRagent_profile
else
	#echo $product_path
	#HLR_delete_line_match /etc/HLRagent_profile  HLRAGENT_PATH=
	HLR_replace_product_path /etc/HLRagent_profile ${product_path}  ${HLRAGENT_PATH}
	echo export HLRAGENT_PATH="$PWD">>/etc/HLRagent_profile
fi

chmod 755 ./Configure/HLRagentd
cp -rf ./Configure/HLRagentd /etc/init.d/
chkconfig --add HLRagentd
chkconfig --level 2345 HLRagentd on


HLR_LINK_FILE=$PWD/hlr.xml
MQTT_INFO_FILE=$PWD/mqttcfg.xml



parameter_value=""

read -p "Input the type of the server[udc|hlr]: " servertype
if [  "udc" = "$servertype" -o  "UDC" = "$servertype" ]; then
   cp -rf ./Configure/mqttcfg-UDC.xml mqttcfg.xml
elif [ "hlr" = "$servertype" -o  "HLR" = "$servertype" ]; then
   cp -rf ./Configure/mqttcfg-HLR.xml mqttcfg.xml
else
	exit 1
fi
#----mqtt ip
HLR_get_parameter_value $MQTT_INFO_FILE HostIp
read -p "Input IP of mq server [$parameter_value]: " mqserver_ip
if [ -z "$mqserver_ip" ]; then
	mqserver_ip=$parameter_value	
fi
mqserver_ip=`echo "$mqserver_ip" | grep "^[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}$"`
if [ -n "$mqserver_ip" ]; then
	HLR_replace_parameter_value $MQTT_INFO_FILE HostIp $parameter_value $mqserver_ip
fi
#----mqtt port
HLR_get_parameter_value $MQTT_INFO_FILE HostPort
read -p "Input PORT of mq server [$parameter_value]: " mqserver_port
if [ -z "$mqserver_port" ]; then
	server_port=$parameter_value	
fi
if [ -n "$mqserver_port" ]; then
	HLR_replace_parameter_value $MQTT_INFO_FILE HostPort $parameter_value $mqserver_port
fi
#----mqtt username
HLR_get_parameter_value $MQTT_INFO_FILE UserName
read -p "Input UserName of mq server [$parameter_value]: " mqserver_user
if [ -z "$mqserver_user" ]; then
	server_port=$parameter_value	
fi
if [ -n "$mqserver_user" ]; then
	HLR_replace_parameter_value $MQTT_INFO_FILE UserName $parameter_value $mqserver_user
fi

#----mqtt password
HLR_get_parameter_value $MQTT_INFO_FILE Password
read -p "Input password of mq server [$parameter_value]: " mqserver_pwd
if [ -z "$mqserver_pwd" ]; then
	server_port=$parameter_value	
fi
if [ -n "$mqserver_pwd" ]; then
	HLR_replace_parameter_value $MQTT_INFO_FILE Password $parameter_value $mqserver_pwd
fi
#----mqtt clientid
HLR_get_parameter_value $MQTT_INFO_FILE ClientID
read -p "Input clientID of mq server [$parameter_value]: " mqserver_clientid
if [ -z "$mqserver_clientid" ]; then
	server_port=$parameter_value	
fi
if [ -n "$mqserver_clientid" ]; then
	HLR_replace_parameter_value $MQTT_INFO_FILE ClientID $parameter_value $mqserver_clientid
fi


#--------------Local ip--------------
HLR_get_parameter_value $HLR_LINK_FILE LocalIp
read -p "Input IP of Local [$parameter_value]: " local_ip
if [ -z "$local_ip" ]; then
	local_ip=$parameter_value	
fi
local_ip=`echo "$local_ip" | grep "^[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}$"`
if [ -n "$local_ip" ]; then
	HLR_replace_parameter_value $HLR_LINK_FILE LocalIp $parameter_value $local_ip
fi

HLR_get_parameter_value $HLR_LINK_FILE LocalPort
read -p "Input PORT of Local [$parameter_value]: " local_port
if [ -z "$local_port" ]; then
	server_port=$parameter_value	
fi
if [ -n "$local_port" ]; then
	HLR_replace_parameter_value $HLR_LINK_FILE LocalPort $parameter_value $local_port
fi


#--------------Peer ip--------------
HLR_get_parameter_value $HLR_LINK_FILE HlrIp
read -p "Input IP of "$servertype" [$parameter_value]: " server_ip
if [ -z "$server_ip" ]; then
	server_ip=$parameter_value	
fi
server_ip=`echo "$server_ip" | grep "^[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}$"`
if [ -n "$server_ip" ]; then
	HLR_replace_parameter_value $HLR_LINK_FILE HlrIp $parameter_value $server_ip
fi

HLR_get_parameter_value $HLR_LINK_FILE HlrPort
read -p "Input PORT of "$servertype" [$parameter_value]: " server_port
if [ -z "$server_port" ]; then
	server_port=$parameter_value	
fi
if [ -n "$server_port" ]; then
	HLR_replace_parameter_value $HLR_LINK_FILE HlrPort $parameter_value $server_port
fi




echo ""
echo "HLRagent parameter configuration finish."
echo ""
exit 1