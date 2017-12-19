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
MAKEFILE_NAME=$(XOSTEST_ROOT)/build/xos_test.mk
#定义本makefile的输出目标文件名，比如hi.o
ifeq ($(MAKE_PLATFORM), VXWORKS)
	PRODUCT_NAME	      = $(XOSTEST_ROOT)/bin/xostest.o
else
	PRODUCT_NAME	      = $(XOSTEST_ROOT)/bin/xostest
endif


OBJS =  $(subst $(subst \,/,$(SRC_ROOT)),$(subst \,/,$(OBJS_DIR)),$(subst \,/,$(TEST_SRC_ROOT)))/test.o
OBJS += $(subst $(subst \,/,$(SRC_ROOT)),$(subst \,/,$(OBJS_DIR)),$(subst \,/,$(XOS_PORT)))/xos_op.o
	
MAKES_LIST = $(TEST_SRC_ROOT)/test.mk
MAKES_LIST += $(XOS_PORT)/xos_op.mak

				
ifeq ($(MAKE_PLATFORM), LINUX)
#测试模块以后有需要新连接的库，可以在这里添加
	LIB_OBJS        = 
#	LIB_OBJS        += -lpthread -ldl -lc -lbfd -pg
endif	

#定义需要链接的库文件
include $(RULES_ROOT)/makefile.rules

include $(RULES_ROOT)/make_link.rule
