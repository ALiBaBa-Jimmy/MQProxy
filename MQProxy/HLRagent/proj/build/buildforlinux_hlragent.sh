#!/bin/bash
export PWD_PRG_ROOT=`pwd`
export TOTAL_BUILD_PRG_ROOT=`pwd`/../..     
export DIR_INSTALL_PACK=$TOTAL_BUILD_PRG_ROOT/../release
export NAME_BIN_PACK=mqtt.tar.gz



if [ ! -n "$1" ] ; then
	echo "Usage: buildforlinux_ehssfe.sh all 0 Version-Name"
	exit -1
fi

#if [ ! -n "$2" ] ; then
#	echo "Usage: buildforlinux_ehssfe.sh modulename[all, youmodulename] errorExportToScreen[0:not export; 1:export] name version"
#	exit -1
#fi

export NAME_INSTALL_PACK=MQTT_INST_$4_PKG
export NAME_INSTALL_PACK_TAR=$NAME_INSTALL_PACK.tar.gz

chmod +x  ./link.sh
dos2unix ./link.sh
./link.sh
function BuildACE()
{
    export BUILD_ACE_ROOT=$TOTAL_BUILD_PRG_ROOT/diameter/ace5.7
    export BUILD_ACE_INCLUDE_PATH=$BUILD_ACE_ROOT/ACE_wrappers 
      
    echo ========================== BUILD_ACE_ROOT=$BUILD_ACE_ROOT===================================================================
    echo ========================== BUILD_ACE_INCLUDE_PATH=$BUILD_ACE_INCLUDE_PATH===================================================================
    echo =========================================================================================================================================
    echo =========================================================================================================================================
    cd $BUILD_ACE_ROOT      
    chmod +x $BUILD_ACE_ROOT/build.sh
    dos2unix $BUILD_ACE_ROOT/build.sh
    
    rm -rf $BUILD_ACE_ROOT/output/*
    cd $BUILD_ACE_ROOT
    $BUILD_ACE_ROOT/build.sh rebuild  
    rm -rf $TOTAL_BUILD_PRG_ROOT/import/ACE_wrappers/bin/linux/lib/*   
    cp -rf $BUILD_ACE_ROOT/output/libACE.so $TOTAL_BUILD_PRG_ROOT/import/ACE_wrappers/bin/linux/lib/
    cp -rf $BUILD_ACE_ROOT/output/libACE-5.7.so $TOTAL_BUILD_PRG_ROOT/import/ACE_wrappers/bin/linux/lib/
    cp -rf $BUILD_ACE_ROOT/output/libACEXML-5.7.so $TOTAL_BUILD_PRG_ROOT/import/ACE_wrappers/bin/linux/lib/
    cp -rf $BUILD_ACE_ROOT/output/libACEXML.so $TOTAL_BUILD_PRG_ROOT/import/ACE_wrappers/bin/linux/lib/
    cp -rf $BUILD_ACE_ROOT/output/libACEXML_Parser-5.7.so $TOTAL_BUILD_PRG_ROOT/import/ACE_wrappers/bin/linux/lib/
    cp -rf $BUILD_ACE_ROOT/output/libACEXML_Parser.so $TOTAL_BUILD_PRG_ROOT/import/ACE_wrappers/bin/linux/lib/
    
    cp -rf $BUILD_ACE_INCLUDE_PATH/ace/ $TOTAL_BUILD_PRG_ROOT/import/ACE_wrappers/include
    cp -rf $BUILD_ACE_INCLUDE_PATH/ACEXML/ $TOTAL_BUILD_PRG_ROOT/import/ACE_wrappers/include 
    
    echo FindFile=$BUILD_ACE_ROOT/output/libACE.so
    LIB_BUILD_SUCCESS=`find $BUILD_ACE_ROOT/output/libACE.so`
    echo LIB_BUILD_SUCCESS=$LIB_BUILD_SUCCESS
    if [ "" != "$LIB_BUILD_SUCCESS" ]; then
        echo =================Build $BUILD_ACE_ROOT SUCCESED=======================
                    	
    else
       echo =================Build $BUILD_ACE_ROOT FAILED=======================
       exit -1
    fi
    cd $PWD_PRG_ROOT   
}


function BuildXos()
{
    export XCPS_ROOT=$TOTAL_BUILD_PRG_ROOT/../xcps 
    export XCPS_ROOT_BUILD_PATH=$XCPS_ROOT/cpsv3000
    export XCPS_LIB_ROOT=$XCPS_ROOT/cpsv3000/lib   
    export RULES_ROOT=$XCPS_ROOT/cpsrules
    export V3000_ROOT=$XCPS_ROOT/cpsv3000
    export PRG_ROOT=`pwd`/../../..   
    export SRC_ROOT=$PRG_ROOT     
    echo prg_root=$PRG_ROOT
    echo src_root=$SRC_ROOT
    export OBJS_DIR=$PRG_ROOT/HLRagent/obj
    mkdir -p $OBJS_DIR
    echo objs_dir=$OBJS_DIR
  
    if [ "$HOSTTYPE" = "x86_64" ];
    then
       echo "x86_64"
       export CPU=x86_64
    else
       echo "i686"
       export CPU=i686
    fi

    export MAKE_PLATFORM=LINUX
    export OBJTYPE=o
    export DEBUG=1
    export OBJTYPE=o
    export MEM_FID_DEBUG=1


    export XOS_MDLMGT=1
    export XOS_LINUX=1
    export XOS_EW_START=1
    export XOS_TELNETD=1
    export XOS_FTP_CLIENT=1
    export XOS_NEED_MAIN=1
    export XOS_NOTC_MEM=1
    export XOS_NEED_OLDXML=1
    export XOS_ModMem_Check=1
    export XOS_ARRAY_DEBUG=1
    export XOS_HASH_DEBUG=1
    export XOS_INPUT_PAR_CHECK=1
   


    export NEED_OAM=1
    export OAM_MSG_FROM_FILE=1
    export NEED_CPPFRM=0
    export XOS_TRACE_AGENT=1
    export XOS_SCTP=1
    export USE_XOS_OAM=1
	export XOS_NEED_HA=1
	export XOS_ATCA=1
    #export ENABLE_PR_XOS_OUT=xoslin
    #export XOS_TIMER_FOURFUNC=1
    echo ========================== XCPS_ROOT_BUILD_PATH=$XCPS_ROOT_BUILD_PATH===================================================================
    echo ========================== XCPS_LIB_ROOT=$XCPS_LIB_ROOT===================================================================
    echo ===========================OBJS_DIR=$OBJS_DIR==================================================================================
    echo =========================================================================================================================================
    cd $XCPS_ROOT_BUILD_PATH    
    echo pwdXCPS_ROOT_BUILD_PATH =`pwd`    
    chmod +x build_linux.sh 
    dos2unix makecps.mak
	rm -rf $OBJS_DIR/*
    make -f $XCPS_ROOT_BUILD_PATH/makecps.mak
    cd $PWD_PRG_ROOT
}


function BuildDiameter()
{
    export BUILD_DIAMETER_ROOT=$TOTAL_BUILD_PRG_ROOT/diameter
    export BUILD_DIAMETER_CODE_PATH=$BUILD_DIAMETER_ROOT/code 
      
    echo ========================== BUILD_DIAMETER_ROOT=$BUILD_DIAMETER_ROOT===================================================================
    echo ========================== BUILD_DIAMETER_CODE_PATH=$BUILD_DIAMETER_CODE_PATH===================================================================
    echo =========================================================================================================================================
    echo =========================================================================================================================================
    cd $BUILD_DIAMETER_ROOT      
    chmod +x $BUILD_DIAMETER_ROOT/makefile
    dos2unix $BUILD_DIAMETER_ROOT/makefile
    
   
    rm -rf $BUILD_DIAMETER_ROOT/bin/libdiameter.so
    make -f $BUILD_DIAMETER_ROOT/makefile
    
    #cp -rf $BUILD_DIAMETER_ROOT/bin/libdiameter.so $TOTAL_BUILD_PRG_ROOT/import/diameter/bin/linux/lib 
    #cp -rf $BUILD_DIAMETER_CODE_PATH/include $TOTAL_BUILD_PRG_ROOT/import/diameter
    
   
    
    echo FindFile=$BUILD_DIAMETER_ROOT/bin/libdiameter.so
    LIB_BUILD_SUCCESS=`find $BUILD_DIAMETER_ROOT/bin/libdiameter.so`
    echo LIB_BUILD_SUCCESS=$LIB_BUILD_SUCCESS
    if [ "" != "$LIB_BUILD_SUCCESS" ]; then
        echo =================Build $BUILD_DIAMETER_ROOT SUCCESED=======================
        rm -rf $TOTAL_BUILD_PRG_ROOT/import/diameter/
        mkdir $TOTAL_BUILD_PRG_ROOT/import/diameter
        mkdir $TOTAL_BUILD_PRG_ROOT/import/diameter/bin
        mkdir $TOTAL_BUILD_PRG_ROOT/import/diameter/bin/linux
        mkdir $TOTAL_BUILD_PRG_ROOT/import/diameter/bin/linux/lib
        cp -f $BUILD_DIAMETER_ROOT/bin/libdiameter.so $TOTAL_BUILD_PRG_ROOT/import/diameter/bin/linux/lib/
        cp -f $BUILD_DIAMETER_ROOT/bin/libdiameter.so $TOTAL_BUILD_PRG_ROOT/hlragent/bin/linux/lib/
        cp -f $TOTAL_BUILD_PRG_ROOT/import/diameter/bin/linux/lib/*  $TOTAL_BUILD_PRG_ROOT/import/diameter/bin/linux      
        cp -rf $BUILD_DIAMETER_CODE_PATH/include/ $TOTAL_BUILD_PRG_ROOT/import/diameter
                    	
    else
       echo =================Build $BUILD_DIAMETER_ROOT FAILED=======================
       exit -1
    fi
    cd $PWD_PRG_ROOT
}
function BuildOneModule()
{                                                                     
     echo =========================================================================================================================================
     echo =========================================================================================================================================
      
    echo 1=$1
    if [ "$1" == "xos" ]; then  
        echo ++++++++ build xos+++++++++++++
        BuildXos
    else 
        if [ "$1" == "diameter" ]; then  
            echo ++++++++ build diameter+++++++++++++
            BuildDiameter    
        else
            if [ "$1" == "ace" ]; then
                echo ++++++++ build ace+++++++++++
                BuildACE
            else
                export TOTAL_BUILD_MODULE_ROOT=$TOTAL_BUILD_PRG_ROOT/$1/proj/build
                export BUILD_MODULE_NAME=$1
                echo ========================== TOTAL_BUILD_MODULE_ROOT=$TOTAL_BUILD_MODULE_ROOT
                echo =========================================================================================================================================
                echo =========================================================================================================================================
                cd $TOTAL_BUILD_MODULE_ROOT
                #rm -rf  $TOTAL_BUILD_MODULE_ROOT/../../obj/*
                #if [ "hlragent" != "$BUILD_MODULE_NAME" ]; then
                     #rm $TOTAL_BUILD_MODULE_ROOT/../../bin/linux/lib/*
                #else
                     #rm $TOTAL_BUILD_MODULE_ROOT/../../bin/linux/spr.out 
                #fi
                
                chmod +x  $TOTAL_BUILD_MODULE_ROOT/build*
                dos2unix $TOTAL_BUILD_MODULE_ROOT/build*
                $TOTAL_BUILD_MODULE_ROOT/buildforlinux.sh $2 $3
                
                if [ "hlragent" != "$BUILD_MODULE_NAME" ]; then
                    echo FindFile=$TOTAL_BUILD_MODULE_ROOT/../../bin/linux/lib/*
                    LIB_BUILD_SUCCESS=`find $TOTAL_BUILD_MODULE_ROOT/../../bin/linux/lib/*`
                    echo LIB_BUILD_SUCCESS=$LIB_BUILD_SUCCESS
                    if [ "" != "$LIB_BUILD_SUCCESS" ]; then
                    	echo =================Build $TOTAL_BUILD_MODULE_ROOT SUCCESED=======================
                    	rm -f ${PWD_PRG_ROOT}/errorbuild_${BUILD_MODULE_NAME}
                    	cd $PWD_PRG_ROOT
                    else
                    	echo =================Build $TOTAL_BUILD_MODULE_ROOT FAILED=======================
                    	cp -f ${TOTAL_BUILD_MODULE_ROOT}/errorbuild_${BUILD_MODULE_NAME} ${PWD_PRG_ROOT}
                    	cd $PWD_PRG_ROOT
                    	exit -1
                    fi
                fi
            fi
        fi
     fi
   
}

function cleanObj()
{  
	echo TOTAL_BUILD_PRG_ROOT=$TOTAL_BUILD_PRG_ROOT
	rm -rf $TOTAL_BUILD_PRG_ROOT/obj/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/hlragent/obj/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/mqtt/obj/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/agentharddb/obj/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/agentdb/obj/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/agentua/obj/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/agentoam/obj/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/dbdll/obj/*
	
	rm -rf $TOTAL_BUILD_PRG_ROOT/mqtt/bin/linux/lib/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/agentdb/bin/linux/lib/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/agentua/bin/linux/lib/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/agentoam/bin/linux/lib/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/agentharddb/bin/linux/lib/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/dbdll/bin/linux/lib/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/hlragent/bin/linux/hlragent.out
	rm -rf $TOTAL_BUILD_PRG_ROOT/hlragent/bin/linux/lib/*
		
	rm -rf $TOTAL_BUILD_PRG_ROOT/publication/bin/linux/lib/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/publication/bin/linux/*so*
	rm -rf $TOTAL_BUILD_PRG_ROOT/publication/hlragent/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/publication/mqtt/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/publication/agentharddb/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/publication/agentdb/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/publication/agentua/*
	rm -rf $TOTAL_BUILD_PRG_ROOT/publication/agentoam/*
	
}
echo param1=$1
echo TOTAL_BUILD_PRG_ROOT=$TOTAL_BUILD_PRG_ROOT
if [ "clean" = "$1"  ]; then
	cleanObj
else	
	if [ "all" = "$1"  ]; then
	   echo =================Build ALL======================= 
	   cleanObj
	   BuildOneModule xos $2 $3 
	   #BuildOneModule dbdll $2 $3
	   BuildOneModule mysqldbdll $2 $3
	   BuildOneModule agentharddb $2 $3
	   BuildOneModule agentua $2 $3
	   BuildOneModule agentoam $2 $3
	   BuildOneModule agentdb $2 $3
	   BuildOneModule mqtt $2 $3
	   BuildOneModule hlragent $2 $3 
	
	   cd $PWD_PRG_ROOT
	   
	   echo "All Done!"	   
	 
	else
	   BuildOneModule $1 $2 $3   
	   
	fi
fi


cd $PWD_PRG_ROOT
exit 0
#cp $TOTAL_BUILD_hlragentPROJ_ROOT/../../bin/$2.tar.gz $PWD_PRG_ROOT/
