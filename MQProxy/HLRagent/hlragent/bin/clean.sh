#!/bin/bash

HLR_DIR=`pwd`/..

rm -f $HLR_DIR/src/xdb/src/xdb_api.c
rm -f $HLR_DIR/src/xdb/src/xdb_oam.c
rm -f $HLR_DIR/src/xdb/src/xdb_smu_group_fun.c
rm -f $HLR_DIR/src/xdb/src/xdb_smu_oss_user_fun.c
rm -f $HLR_DIR/src/xdb/src/xdb_smu_group_status_fun.c
rm -f $HLR_DIR/src/xdb/src/xdb_addr_commander_fun.c
rm -f $HLR_DIR/src/xdb/src/xdb_addr_communction_fun.c
rm -f $HLR_DIR/src/xdb/src/xdb_addr_lost_stat_fun.c
rm -f $HLR_DIR/src/xdb/src/xdb_addr_person_fun.c  
rm -f $HLR_DIR/src/xdb/src/xdb_smu_acl_fun.c
rm -f $HLR_DIR/src/xdb/src/xdb_smu_cpez_mz_fun.c  
rm -f $HLR_DIR/src/xdb/src/xdb_pid_port_uid.c
rm -f $HLR_DIR/src/xdb/src/xdb_smu_cid_fun.c

rm -rf $HLR_DIR/obj/*
rm -f $HLR_DIR/bin/linux/hlr
rm -f $HLR_DIR/bin/linux/*.so
cp $HLR_DIR/bin/linux/backup/*.so $HLR_DIR/bin/linux/


dos2unix $HLR_DIR/src/xdb/src/xdb_api.pc
dos2unix $HLR_DIR/src/xdb/src/xdb_oam.pc
dos2unix $HLR_DIR/src/xdb/src/xdb_smu_group_fun.pc
dos2unix $HLR_DIR/src/xdb/src/xdb_smu_oss_user_fun.pc
dos2unix $HLR_DIR/src/xdb/src/xdb_smu_group_status_fun.pc
dos2unix $HLR_DIR/src/xdb/src/xdb_addr_commander_fun.pc
dos2unix $HLR_DIR/src/xdb/src/xdb_addr_communction_fun.pc
dos2unix $HLR_DIR/src/xdb/src/xdb_addr_lost_stat_fun.pc
dos2unix $HLR_DIR/src/xdb/src/xdb_addr_person_fun.pc  
dos2unix $HLR_DIR/src/xdb/src/xdb_smu_acl_fun.pc
dos2unix $HLR_DIR/src/xdb/src/xdb_smu_cpez_mz_fun.pc  
dos2unix $HLR_DIR/src/xdb/src/xdb_pid_port_uid.pc
dos2unix $HLR_DIR/src/xdb/src/xdb_smu_cid_fun.pc
