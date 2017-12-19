#!/bin/bash

if [ ! -n "$1" ] ; then
	echo "Usage: packet.sh name"
	exit 0
fi

PACKET_DIR=./"$1"
rm -rf $1
rm -rf $1.tar.gz

cp -r ./linux $PACKET_DIR
find $PACKET_DIR -name "vssver.scc" | xargs -i rm -f {}
find $PACKET_DIR/charge_agt_lib -name "*.so" | xargs -i rm -f {}
find $PACKET_DIR/charge_agt_lib -name "*.1" | xargs -i rm -f {}
ls $PACKET_DIR/charge_agt_lib/* | xargs -i chmod u+x {}
ls $PACKET_DIR/*.so | xargs -i chmod u+x {}
chmod u+x $PACKET_DIR/hlragent.out
chmod u+x $PACKET_DIR/*.sh
dos2unix $PACKET_DIR/*.sh 
chmod u+w $PACKET_DIR/cfg.xml
chmod u+w $PACKET_DIR/xos.xml
chmod u+w $PACKET_DIR/*.xml
dos2unix $PACKET_DIR/script/*.sh
chmod  u+x $PACKET_DIR/script/*.sh
#ls $PACKET_DIR/script/* | xargs -i chmod u+x {}
#ls $PACKET_DIR/script/* | xargs -i dos2unix {}

tar -czvf $PACKET_DIR.tar.gz $PACKET_DIR

