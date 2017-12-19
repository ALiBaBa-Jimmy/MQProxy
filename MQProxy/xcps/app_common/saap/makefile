########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################
#包含其它makefile文件，比如include $(SRC_ROOT)/protocol/h248/mgco.dir
#用户可以将复杂的头文件/源文件目录、编译宏放在单独的文件中，在此包含。
#然后在后面引用其中的变量
include $(XCPS_ROOT)/app_common/xcps.dir

#设置本makefile文件的类型，值包括 SRC_TYPE 或 LINK_TYPE
MAKE_TYPE             = LINK_TYPE

MAKEFILE_NAME         = $(CMM_SAAP)/makefile
PRODUCT_NAME          = saapall.o

#OBJS  =    $(OBJS_DIR)/xcps/cpsv3000/xos.$(OBJTYPE)

OBJS = $(OBJS_DIR)/xcps/app_common/saap/hashlst/hashlst.o \
	    $(OBJS_DIR)/xcps/app_common/saap/tcpe/tcpe.o \
	    $(OBJS_DIR)/xcps/app_common/saap/saap/saap.o 
	         

#MAKES_LIST  = $(V3000_ROOT)/makefile


 MAKES_LIST  =   $(CMM_SAAP)/hashlst/makefile \
               $(CMM_SAAP)/tcpe/makefile \
               $(CMM_SAAP)/saap/makefile

              
LIB_PATH     =

#包含链接类型的规则文件
include $(RULES_ROOT)/make_link.rule


########################################################################
# Check definitions and include rules  
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

