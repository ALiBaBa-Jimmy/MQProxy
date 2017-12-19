########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################

#包含其它makefile文件，比如include $(SRC_ROOT)/protocol/h248/mgco.dir
#用户可以将复杂的头文件/源文件目录、编译宏放在单独的文件中，在此包含。
#然后在后面引用其中的变量
include $(XCPS_ROOT)/app_common/xcps.dir
include $(V3000_ROOT)/xos.dir

# 设置本makefile文件的类型，值包括 SRC_TYPE 或 LINK_TYPE
MAKE_TYPE             = SRC_TYPE

#是否需要支持Trillium SSI的规则，值为YES表示需要。
NEED_TRI_SSI	      = ###目前不需要

#定义本makefile的输出目标文件名，比如hi.o
PRODUCT_NAME	      = xosvx.o

#定义本makefile的路径名，用于计算路径，比如 $(SRC_ROOT)/protocol/tucl/bdy/makefile
MAKEFILE_NAME         = $(XOS_OS)/vxworks/makefile

ifeq ($(MAKE_TYPE), SRC_TYPE)
#用于源类型的makefile

#设置源文件的路径，比如$(SRC_ROOT)/protocol/tucl/bdy/src
  VPATH               = $(XOS_OS)/vxworks/src
  
#设置源文件列表，用户可以指定，或通过缺省方式找出源路径下的所有源文件（.c/.cpp/.s）。  
  SRCS                = $(wildcard $(foreach EVERY_VPATH,$(VPATH),$(EVERY_VPATH)/*.c $(EVERY_VPATH)/*.cpp $(EVERY_VPATH)/*.s))

#定义用户特定的头文件搜索路径，格式：-I路径名，比如-I$(SRC_ROOT)/protocol/tucl/bdy/inc
#如果很多，可以用换行符号“\”连接
  EXTRA_INCLUDE	    = $(XOS_INC)

#定义用户特定的编译选项，格式：-D编译宏，比如-DGCP_ASN。
  EXTRA_DEFINE      = #$(ST_MAC)	###在XXX.MAC中定义？

else
#用于链接类型的makefile

#定义目标文件的路径名，比如$(subst $(SRC_ROOT),$(OBJS_DIR),$(MAKEFILE_DIR))/bdy/hi.o
#$(subst $(SRC_ROOT),$(OBJS_DIR),$(MAKEFILE_DIR))用于计算出目标文件的存放路径
#可以通过“\”换行符连接多个路径名
#可以通过“OBJS += ”方式增加路径名。
OBJS             = 

#定义对应的makefile文件的路径名，比如$(MAKEFILE_DIR)/bdy/makefile
#通过本makefile文件可以创建OBJS中定义的目标文件。
MAKES_LIST       = 

#定义需要连接的库文件路径
LIB_PATH            =

#包含链接类型的规则文件
include $(RULES_ROOT)/make_link.rule

endif

#定义需要链接的库文件
LIB_OBJS              = 


########################################################################
# Check common definitions and include rules  
########################################################################
#检查源文件的根路径是否定义
ifndef SRC_ROOT
exit_to_setenv:
	@echo Must define SRC_ROOT!
else
#检查目标文件的根路径是否定义
ifndef OBJS_DIR
exit_to_setenv:
	@echo Must define OBJS_DIR!
else
#检查CPU是否定义
ifndef CPU
exit_to_setenv:
	@echo Must define CPU!
else
#包含makefile的平台规则文件。
   include $(RULES_ROOT)/makefile.rules
endif
endif
endif

########################################################################
# targets definition
########################################################################

#包含源类型的规则文件
ifeq ($(MAKE_TYPE), SRC_TYPE)
  include $(RULES_ROOT)/make_src.rule
endif

