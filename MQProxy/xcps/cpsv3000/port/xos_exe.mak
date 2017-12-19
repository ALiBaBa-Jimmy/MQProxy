########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################

include $(V3000_ROOT)/xos.dir

########################################################################
# Set makefile's type (SRC_TYPE or LINK_TYPE) (you must set it)
########################################################################
MAKE_TYPE             = LINK_TYPE
NEED_TRI_SSI	      = ###

ifdef XOS_SET_OBJ_DIR_AS_OUT
XOS_OUT_DIR = $(OBJS_DIR)/$(ENABLE_PR_XOS_OUT)/debug
else
XOS_OUT_DIR = $(OBJS_DIR)/debug
endif


#以下是用来把“.xml”文件拷贝至debug目录下

ifeq ("$(MAKE_PLATFORM)", "WIN32")
.PHONY:mkcpwin
default : mkcpwin
mkcpwin:
	@IF NOT EXIST $(subst /,\,$(OBJS_DIR)\debug) (mkdir $(subst /,\,$(OBJS_DIR)\debug))
	$(CP) $(subst /,\,$(XOS_PORT)\*.xml) $(subst /,\,$(OBJS_DIR)\debug)
PRODUCT_NAME	      = $(XOS_OUT_DIR)/$(ENABLE_PR_XOS_OUT).exe	#测试桩所生成.exe文件的路径和名称
endif

ifeq ("$(MAKE_PLATFORM)", "VXWORKS")
.PHONY:mkcpwin
default : mkcpwin
mkcpwin:
	@IF NOT EXIST $(subst /,\,$(OBJS_DIR)\debug) (mkdir $(subst /,\,$(OBJS_DIR)\debug))
	$(CP) $(subst /,\,$(XOS_PORT)\*.xml) $(subst /,\,$(OBJS_DIR)\debug)
PRODUCT_NAME	      = $(XOS_OUT_DIR)/$(ENABLE_PR_XOS_OUT)	#测试桩所生成  文件的路径和名称
endif

ifeq ("$(MAKE_PLATFORM)", "LINUX")
.PHONY:mkcplin
default : mkcplin
mkcplin:
	@-mkdir -m 700 -p $(subst \,/,$(OBJS_DIR)/debug)
	@$(CP) $(XOS_PORT)/*.xml $(OBJS_DIR)/debug
PRODUCT_NAME	      = $(XOS_OUT_DIR)/$(ENABLE_PR_XOS_OUT)	#测试桩所生成可执行文件的路径和名称
endif

ifeq ("$(MAKE_PLATFORM)", "SOLARIS")
.PHONY:mkcpsol
default : mkcpsol
mkcpsol:
	@-mkdir -m 700 -p $(subst \,/,$(OBJS_DIR)/debug)
	@$(CP) $(XOS_PORT)/*.xml $(OBJS_DIR)/debug
PRODUCT_NAME	      = $(XOS_OUT_DIR)/$(ENABLE_PR_XOS_OUT)	#测试桩所生成可执行文件的路径和名称
endif

#=============================================================

MAKEFILE_NAME         = $(XOS_PORT)/xos_exe.mak


ifeq ($(MAKE_TYPE), SRC_TYPE)
  VPATH               = 
  SRCS                = $(wildcard $(foreach EVERY_VPATH,$(VPATH),$(EVERY_VPATH)/*.c $(EVERY_VPATH)/*.cpp $(EVERY_VPATH)/*.s))
  EXTRA_INCLUDE	      = 
  EXTRA_DEFINE        = 
else
  LIB_PATH            = $(V3000_ROOT)
  LIBS_DIR            = #$(subst /,\,$(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_PATH)))
  
###第一次引用 V3000_ROOT ，先把'\'转成'/'以匹配 SRC_ROOT  
  LIB_SRC_DIR					= $(subst \,/,$(LIB_PATH))
  
  
ifeq ("$(MAKE_PLATFORM)", "WIN32")
  OBJS	              = \
  	                    $(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/port/xos_op.$(OBJTYPE) \
											$(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/xosport.o	#main函数生成的目标文件名
        	               
  MAKES_LIST    	  = \
        	            $(XOS_PORT)/xos_op.mak  \
            	       	$(V3000_ROOT)/makefile.mak

ifeq ($(XOS_MDLMGT), 1)
	OBJS		         += \
	                    $(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/bdy/xosroot.o
	OBJS		         += \
	                    $(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/bdy/xoscfg.o
endif
endif
ifeq ("$(MAKE_PLATFORM)", "LINUX")
  OBJS	              = \
  	                    $(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/port/xos_op.$(OBJTYPE) #main函数生成的目标文件名
        	               
  MAKES_LIST    	  = \
        	            $(XOS_PORT)/xos_op.mak
endif
ifeq ("$(MAKE_PLATFORM)", "SOLARIS")
  OBJS	              = \
  	                    $(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/port/xos_op.$(OBJTYPE) #main函数生成的目标文件名
        	               
  MAKES_LIST    	  = \
        	            $(XOS_PORT)/xos_op.mak
endif
ifeq ("$(MAKE_PLATFORM)", "VXWORKS")
  OBJS	              = \
  	                    $(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/port/xos_op.$(OBJTYPE) #main函数生成的目标文件名
        	               
  MAKES_LIST    	  = \
        	            $(XOS_PORT)/xos_op.mak
endif

export GLOBAL_MACRO +=
include $(RULES_ROOT)/make_link.rule

endif
ifeq ($(MAKE_PLATFORM), WIN32)
	LIB_OBJS              = ws2_32.lib  
else
	LIB_OBJS              = 
endif

########################################################################
# root directory (you must set it)
# if you source code is not in current directory, you must set VPATH
########################################################################

########################################################################
# include rules and common directory definition
########################################################################
#-----------------------------------------------------------------------
ifndef SRC_ROOT
exit_to_setenv:
	@echo Must define SRC_ROOT!
else
ifndef SRC_ROOT
exit_to_setenv:
	@echo Must define SRC_ROOT!
else
ifndef CPU
exit_to_setenv:
	@echo Must define CPU!
else
   include $(RULES_ROOT)/makefile.rules
endif
endif
endif
#-----------------------------------------------------------------------

########################################################################
# (you could add your product name in PRODUCT_NAME)
# (you could add your source code in SRCS)
# (you could add your object in OBJS)
# (you could add depend in DEPEND)
########################################################################

########################################################################
# targets definition
########################################################################


