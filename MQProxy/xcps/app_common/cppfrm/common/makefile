########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################

########################################################################
# Set makefile's type (SRC_TYPE-default or LINK_TYPE) (you must set it)
########################################################################

include $(XCPS_ROOT)/app_common/xcps.dir

MAKE_TYPE            = SRC_TYPE
PRODUCT_NAME	     = cppfrm_common.o
MAKEFILE_NAME        = $(SRC_CPPFRM_ROOT)/common/makefile
VPATH                = $(SRC_CPPFRM_ROOT)/common/src
SRCS                 = $(wildcard $(foreach EVERY_VPATH,$(VPATH),$(EVERY_VPATH)/*.c $(EVERY_VPATH)/*.cpp $(EVERY_VPATH)/*.s))
EXTRA_INCLUDE	     = -I$(SRC_CPPFRM_ROOT)/common/inc \
                       -I$(SRC_CPPFRM_ROOT)/stl/inc \
                       -I$(XOS_INC)

EXTRA_DEFINE         = 
LIB_OBJS             = 

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
# If make source files include make source rules
########################################################################

ifeq ($(MAKE_TYPE), SRC_TYPE)
  include $(RULES_ROOT)/make_src.rule      
endif
