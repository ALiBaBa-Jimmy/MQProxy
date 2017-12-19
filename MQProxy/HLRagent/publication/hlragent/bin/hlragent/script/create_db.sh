#!/bin/bash



echo create db begin...
export HLRAGENT_PATH=`pwd`
export HLRAGENT_DB_SQL=$HLRAGENT_PATH/create_udc.sql


mysql  -uroot -pxinwei -hlocalhost <${HLRAGENT_DB_SQL}

echo create db end...
