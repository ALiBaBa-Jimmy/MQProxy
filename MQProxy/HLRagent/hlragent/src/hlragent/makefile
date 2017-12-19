########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################


########################################################################
# Set makefile's type (SRC_TYPE-default or LINK_TYPE) (you must set it)
########################################################################
MAKE_TYPE           = SRC_TYPE
PRODUCT_NAME        = $(BUILD_MODULE_NAME).o
MAKEFILE_NAME       = $(SRC_ROOT)/hlragent/makefile 
VPATH               = $(subst makefile,src,$(MAKEFILE_NAME))
SRCS                = $(wildcard $(foreach EVERY_VPATH,$(VPATH),$(EVERY_VPATH)/*.c $(EVERY_VPATH)/*.cpp $(EVERY_VPATH)/*.s))          
EXTRA_INCLUDE      = -I$(subst makefile,inc,$(MAKEFILE_NAME))\
                     -I$(ACE_INCLUDE_ROOT)\
                     -I$(XCPS_ROOT)/app_common/oam/inc \
                     -I$(SRC_ROOT)/mqtt/inc \
					 -I$(SRC_ROOT)/agentoam/inc \
					 -I$(SRC_ROOT)/agenttrace/inc \
					 -I$(SRC_ROOT)/agentdb/inc \
					 -I$(SRC_ROOT)/agentua/inc \
					 -I$(SRC_ROOT)/agenthlr/inc \
                     -I$(XCPS_INCLUDE_ROOT)\
					 -I$(COMMON_DIR)/common/inc  \
					 -I$(AGENTHARDDB_INCLUDE_ROOT_PATH)/agentharddb \
					 -I$(PARENT_PRG_ROOT)/mqtt/include/mqtt \
                     -I$(XCPS_ROOT)/cpsv3000/port

EXTRA_DEFINE         =


LIB_OBJS  = 
              

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
   include $(RULES_ROOT)/makefile.rules
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
