#!/bin/bash
#auth: maliming
#date:2013-8-22

function usage()
{
	echo -e "\nArgument error!"
	echo -e "Plese input: $0 [ exe [ -E / -S ] / lib [ a / so ] / clean [ exe / lib ] ]"
	echo -e "exe    : output execute binary file!"
	echo -e "   -E  : output extra precompile file!"
	echo -e "   -S  : output extra assembly file!"
	echo -e "lib    : output library file!"
	echo -e "   a   : output static library file!"
	echo -e "   so  : output shared library file!"
	echo -e "clean  : clean output file!"
	echo -e "   exe : clean all output intermediate and target file!"
	echo -e "   lib : clean all output lib and target file!"
	exit 1
}

if [ $# != 1 ] && [ $# != 2 ]; then
	usage
fi

if [ "$HOSTTYPE" = "x86_64" ];
then
   echo "x86_64"
   export CPU=x86_64
else
   echo "i686"
   export CPU=i686
fi

export MAKE_PLATFORM=LINUX
export PRODUCT=XW_SS
export MICR_BASE=/usr
export SRC_ROOT=$PWD/../..
echo "current SRC_ROOT=$SRC_ROOT"

export XCPS_ROOT=$SRC_ROOT
export V3000_ROOT=$XCPS_ROOT/cpsv3000
export RULES_ROOT=$XCPS_ROOT/cpsrules
export LIB_DIR=$V3000_ROOT/lib
export OBJS_DIR=$V3000_ROOT/obj/linux
mkdir -p $OBJS_DIR

export ENABLE_PR_XOS_OUT=xoslin.out
export WARNING=1
export DEBUG=1
export OBJTYPE=o
export XOS_NEED_MAIN=1
export XOS_MDLMGT=1
export XOS_TELNETD=1
export XOS_HIGHTIMER=1
export XOS_ModMem_Check=1
export XOS_HASH_DEBUG=1
export XOS_ARRAY_DEBUG=1
export INPUT_PAR_CHECK=1
export XOS_LINUX=1
export XOS_FTP_CLIENT=1
export BIG_POLLING_FD_SETSIZE=1
export XOS_EW_START=1
export XOS_SCTP=1
export XOS_ENABLE_GPROF=0
export XOS_ENABLE_GCOV=0
export XOS_UDT_MODULE=0
export XOS_NEED_IPC=0
export MEM_FID_DEBUG=0
export GCC_STD_CPP11=0

if [ "$1" == "exe" ]; then
	if [ $# == 2 ]; then
		if [ "$2" == "-E" ]; then
			export XOS_EXPORT_E_FILE=1
		elif [ "$2" == "-S" ]; then
			export XOS_EXPORT_S_FILE=1
		else
			usage
		fi
	fi
	make -C $OBJS_DIR -f $V3000_ROOT/port/xos_exe.mak
	
	rm -rf $OBJS_DIR/debug/start_xos.sh
	echo "#!/bin/bash" >> $OBJS_DIR/debug/start_xos.sh
	echo "path=""$""PWD" >> $OBJS_DIR/debug/start_xos.sh
	echo "export XOS_PATH_ENV=""$""path""/xoslin.out" >> $OBJS_DIR/debug/start_xos.sh
	echo "ulimit -c unlimited"  >> $OBJS_DIR/debug/start_xos.sh
	echo 'sysctl -w kernel.core_pattern=core.%p.%e.%t' >> $OBJS_DIR/debug/start_xos.sh
	echo "$""path/xoslin.out -D" >> $OBJS_DIR/debug/start_xos.sh
	
	chmod a+x $OBJS_DIR/debug/start_xos.sh
	
	rm -rf $OBJS_DIR/debug/stop_xos.sh
	echo "#!/bin/bash" >> $OBJS_DIR/debug/stop_xos.sh
	echo "curpwd=""$""PWD" >> $OBJS_DIR/debug/stop_xos.sh
	echo "XOS_PID=\`ps -ef |grep xoslin.out | grep \$curpwd | awk '{print \$2}'\`;" >> $OBJS_DIR/debug/stop_xos.sh
	echo "if test -z ""$""{XOS_PID}" >> $OBJS_DIR/debug/stop_xos.sh
	echo "then" >> $OBJS_DIR/debug/stop_xos.sh
	echo "	echo ""$""curpwd/xoslin.out \"not start\";" >> $OBJS_DIR/debug/stop_xos.sh
	echo "	exit;" >> $OBJS_DIR/debug/stop_xos.sh
	echo "fi" >> $OBJS_DIR/debug/stop_xos.sh
	
	echo "[ \"\" != \"\${XOS_PID}\" ] && kill -9 \${XOS_PID};" >> $OBJS_DIR/debug/stop_xos.sh
	echo "echo stop \$curpwd/xoslin.out;" >> $OBJS_DIR/debug/stop_xos.sh
	
	chmod a+x $OBJS_DIR/debug/stop_xos.sh

	if [ "$XOS_ENABLE_GCOV" == "1" ]; then
	    rm -rf $OBJS_DIR/debug/gcov.sh
	    echo "#!/bin/bash" >> $OBJS_DIR/debug/gcov.sh
	    echo 'if [ "$XCPS_ROOT" = "" ] ;then' >> $OBJS_DIR/debug/gcov.sh
	    echo '  XCPS_ROOT=../../../..' >> $OBJS_DIR/debug/gcov.sh
	    echo 'fi' >> $OBJS_DIR/debug/gcov.sh
 	    echo 'chmod +x $XCPS_ROOT/tools/lcov/*' >> $OBJS_DIR/debug/gcov.sh		
	    echo 'LCOV=$XCPS_ROOT/tools/lcov/lcov' >> $OBJS_DIR/debug/gcov.sh
	    echo 'GENHTML=$XCPS_ROOT/tools/lcov/genhtml' >> $OBJS_DIR/debug/gcov.sh
	    echo 'if [ "$1" == "clean" ] ;then' >> $OBJS_DIR/debug/gcov.sh
	    echo '  rm -f ./gcov.info' >> $OBJS_DIR/debug/gcov.sh
	    echo '  rm -rf ./gcov' >> $OBJS_DIR/debug/gcov.sh
	    echo '  $LCOV --zerocounters --directory $XCPS_ROOT' >> $OBJS_DIR/debug/gcov.sh
	    echo 'else' >> $OBJS_DIR/debug/gcov.sh
	    echo '  GCOV_PATH=$(cd ../../../..; pwd)' >> $OBJS_DIR/debug/gcov.sh
	    echo '  $LCOV  --capture --directory $GCOV_PATH  --output-file gcov.info --test-name test_noargs --no-external' >> $OBJS_DIR/debug/gcov.sh
	    echo '  echo -e "---------------------------------lcov end! genhtml begin--------------------------------\n"' >> $OBJS_DIR/debug/gcov.sh
	    echo '  $GENHTML gcov.info  --output-directory ./gcov  --title "XOS GCOV" --show-details --legend' >> $OBJS_DIR/debug/gcov.sh
	    echo 'fi' >> $OBJS_DIR/debug/gcov.sh
	
	    chmod a+x $OBJS_DIR/debug/gcov.sh
	fi
  	echo "-----------------------------------------------------------------------------------------------"
  	echo "Build over!"
elif [ "$1" == "lib" ]; then
	if [ $# == 2 ]; then
		if [ "$2" == "a" ]; then
			export OBJTYPE=a
		elif [ "$2" == "so" ]; then
			export OBJTYPE=so
		else
			usage
		fi
	else
		usage
	fi
	
	mkdir -p $LIB_DIR
	make -C $OBJS_DIR -f $V3000_ROOT/port/xos_op.mak
	cp $OBJS_DIR/cpsv3000/port/xos_op.$OBJTYPE $LIB_DIR/libxos_op.$OBJTYPE
	echo "Build over!"
elif [ "$1" == "clean" ]; then
	if [ $# == 2 ]; then
		if [ "$2" == "lib" ]; then
			rm -rf $LIB_DIR
		elif [ "$2" != "exe" ]; then
			usage
		fi
	fi
	
	rm -rf ../obj/*

	if [ "$XOS_ENABLE_GCOV" == "1" ] && [ -f $OBJS_DIR/debug/gcov.sh ] ;then
		$OBJS_DIR/debug/gcov.sh clean
	fi
	echo "clean complete!"
	#make -C $OBJS_DIR -f  $V3000_ROOT/port/xos_exe.mak clean
else
	usage
fi