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
PRODUCT_NAME	      = xos_op.$(OBJTYPE)

MAKEFILE_NAME         = $(XOS_PORT)/xos_op.mak


ifeq ($(MAKE_TYPE), SRC_TYPE)
  VPATH               = 
  SRCS                = $(wildcard $(foreach EVERY_VPATH,$(VPATH),$(EVERY_VPATH)/*.c $(EVERY_VPATH)/*.cpp $(EVERY_VPATH)/*.s))
  EXTRA_INCLUDE	      = 
  EXTRA_DEFINE        = 
else
  LIB_PATH            = $(V3000_ROOT)

###第一次引用 V3000_ROOT ，先把'\'转成'/'以匹配 SRC_ROOT
	LIB_SRC_DIR					= $(subst \,/,$(LIB_PATH))

ifeq ("$(MAKE_PLATFORM)", "VXWORKS")
	OBJS		     =  $(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/xos.o \
						$(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/prt.o
else
	OBJS		     =  $(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/xos.$(OBJTYPE) \
						$(subst $(SRC_ROOT),$(OBJS_DIR),$(LIB_SRC_DIR))/prt.o
endif
			
MAKES_LIST       =  $(V3000_ROOT)/makefile  \
					$(V3000_ROOT)/makefile.mak


export GLOBAL_MACRO +=
include $(RULES_ROOT)/make_link.rule

endif

LIB_OBJS              = 

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
ifndef OBJS_DIR
exit_to_setenv:
	@echo Must define OBJS_DIR!
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

