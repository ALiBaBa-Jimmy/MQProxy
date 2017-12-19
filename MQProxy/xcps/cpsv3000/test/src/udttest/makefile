########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################

#用户可以将复杂的头文件/源文件目录、编译宏放在单独的文件中，在此包含。
#然后在后面引用其中的变量

MAKE_TYPE=SRC_TYPE
PRODUCT_NAME=udttest.o
MAKEFILE_NAME=$(TEST_SRC_ROOT)/udttest/makefile

VPATH               = $(TEST_SRC_ROOT)/udttest

#设置源文件列表，用户可以指定，或通过缺省方式找出源路径下的所有源文件（.c/.cpp/.s）。  
SRCS                = $(wildcard $(foreach EVERY_VPATH, $(VPATH), $(EVERY_VPATH)/*.cpp $(EVERY_VPATH)/*.c))

#定义用户特定的头文件搜索路径，格式：-I路径名，比如-I$(SRC_ROOT)/protocol/tucl/bdy/inc
#如果很多，可以用换行符号“\”连接
EXTRA_INCLUDE += -I$(V3000_ROOT) -I$(V3000_ROOT)/common -I$(V3000_ROOT)/bdy/inc -I$(V3000_ROOT)/port
EXTRA_INCLUDE += -I$(V3000_ROOT)/udt/src

#定义用户特定的编译选项，格式：-D编译宏，比如-DGCP_ASN。
ifeq ($(MAKE_PLATFORM), VXWORKS)
	EXTRA_INCLUDE += -I$(V3000_ROOT)/os/vxworks/inc 
	EXTRA_DEFINE     += -DXOS_VXWORKS
else
	EXTRA_INCLUDE += -I$(V3000_ROOT)/os/linux/inc 
	EXTRA_DEFINE     += -DXOS_LINUX
endif


include $(RULES_ROOT)/make_src.rule

include $(RULES_ROOT)/makefile.rules


