########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################

#用户可以将复杂的头文件/源文件目录、编译宏放在单独的文件中，在此包含。
#然后在后面引用其中的变量
include $(XCPS_ROOT)/app_common/xcps.dir
include $(XCPS_ROOT)/cpsv3000/xos.dir

MAKE_TYPE  = LINK_TYPE
MAKEFILE_NAME=$(TEST_SRC_ROOT)/test.mk
#定义本makefile的输出目标文件名，比如hi.o
PRODUCT_NAME   = test.o


SRC_OBJ_DIR=$(subst $(subst \,/,$(SRC_ROOT)),$(subst \,/,$(OBJS_DIR)),$(subst \,/,$(TEST_SRC_ROOT)))

#OBJS = $(SRC_OBJ_DIR)/timer/timer.o 
OBJS = \
		$(SRC_OBJ_DIR)/msgtrace/msgtrace.o \
		$(SRC_OBJ_DIR)/ntltest/ntltest.o  \
		$(SRC_OBJ_DIR)/gprof/gproftest.o \
		$(SRC_OBJ_DIR)/md5/md5test.o \
		$(SRC_OBJ_DIR)/ftptest/ftptest.o \
		$(SRC_OBJ_DIR)/memtest/memtest.o \
		$(SRC_OBJ_DIR)/nfstest/nfstest.o \
		$(SRC_OBJ_DIR)/ntl_server/ntlserver.o \
		$(SRC_OBJ_DIR)/ntl_client/ntlclient.o \
		$(SRC_OBJ_DIR)/snprintf_test/snprintf_test.o \
		$(SRC_OBJ_DIR)/signal_test/signaltest.o \
		$(SRC_OBJ_DIR)/sctp_server/sctpserver.o \
		$(SRC_OBJ_DIR)/sctp_client/sctpclient.o \
		$(SRC_OBJ_DIR)/udp_server/udpserver.o \
		$(SRC_OBJ_DIR)/autotest/autotest.o \
		$(SRC_OBJ_DIR)/log_client/logclient.o \
		$(SRC_OBJ_DIR)/ta_ser/ta_ser.o \
		$(SRC_OBJ_DIR)/ta_cli/ta_cli.o
		

#MAKES_LIST = $(TEST_SRC_ROOT)/timer/makefile 
MAKES_LIST = \
			  $(TEST_SRC_ROOT)/msgtrace/makefile \
  			  $(TEST_SRC_ROOT)/snprintf_test/makefile \
			  $(TEST_SRC_ROOT)/ntltest/makefile \
			  $(TEST_SRC_ROOT)/md5/makefile \
			  $(TEST_SRC_ROOT)/gprof/makefile \
			  $(TEST_SRC_ROOT)/ftptest/makefile \
			  $(TEST_SRC_ROOT)/memtest/makefile \
			  $(TEST_SRC_ROOT)/nfstest/makefile \
			  $(TEST_SRC_ROOT)/ntl_server/makefile \
			  $(TEST_SRC_ROOT)/ntl_client/makefile \
			  $(TEST_SRC_ROOT)/sctp_server/makefile \
			  $(TEST_SRC_ROOT)/sctp_client/makefile \
			  $(TEST_SRC_ROOT)/udp_server/makefile \
			  $(TEST_SRC_ROOT)/autotest/makefile    \
			  $(TEST_SRC_ROOT)/signal_test/makefile \
			  $(TEST_SRC_ROOT)/log_client/makefile \
			  $(TEST_SRC_ROOT)/ta_ser/makefile \
			  $(TEST_SRC_ROOT)/ta_cli/makefile

#定义需要链接的库文件
include $(RULES_ROOT)/makefile.rules

include $(RULES_ROOT)/make_link.rule
