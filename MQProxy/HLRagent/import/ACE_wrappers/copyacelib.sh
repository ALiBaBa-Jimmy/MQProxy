#!/bin/bash

export PROJ_PATH=`pwd`
export ACE_ROOT=$PROJ_PATH/ACE_wrappers 
echo ACE_ROOT=$ACE_ROOT

mkdir $PROJ_PATH/lib
cp $ACE_ROOT/ace/Compression/libACE_Compression.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/ace/ETCL/libACE_ETCL_Parser.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/ace/ETCL/libACE_ETCL.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/ace/Monitor_Control/libACE_Monitor_Control.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/ace/Compression/rle/libACE_RLECompression.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/ace/libACE.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/ACEXML/parser/parser/libACEXML_Parser.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/ACEXML/common/libACEXML.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/ACEXML/apps/svcconf/libACEXML_XML_Svc_Conf_Parser.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/ASNMP/asnmp/libasnmp.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/apps/Gateway/Peer/libGateway_Peer.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/apps/Gateway/Gateway/libGateway.so.5.7.0 $PROJ_PATH/lib/


cp $ACE_ROOT/apps/JAWS2/HTTPU/libHTTPU.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/apps/JAWS2/JAWS/libJAWS2.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/apps/JAWS3/jaws3/libJAWS3.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/apps/JAWS/server/libJAWS.so.5.7.0 $PROJ_PATH/lib/
cp $ACE_ROOT/Kokyu/libKokyu.so.5.7.0 $PROJ_PATH/lib/

tar -czvf acelib.tar.gz $PROJ_PATH/lib/
