########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################
include $(XCPS_ROOT)/app_common/xcps.dir
include $(V3000_ROOT)/xos.dir

########################################################################
# Set makefile's type (SRC_TYPE-default or LINK_TYPE) (you must set it)
########################################################################
MAKE_TYPE           = LINK_TYPE

ifeq ($(MAKE_PLATFORM), WIN32)
  PRODUCT_NAME       = 
else
  ifeq ($(MAKE_PLATFORM), LINUX)
    PRODUCT_NAME          = $(BUILD_MODULE_OUTPUT)
  endif
endif
MAKEFILE_NAME       = $(SRC_ROOT)/$(BUILD_MODULE_NAME).mak


ifeq ($(MAKE_TYPE), SRC_TYPE)
  VPATH                = 
  SRCS                 = $(wildcard $(foreach EVERY_VPATH,$(VPATH),$(EVERY_VPATH)/*.c $(EVERY_VPATH)/*.cpp $(EVERY_VPATH)/*.s))
  EXTRA_DEFINE         = 
  EXTRA_INCLUDE        = 
else 
  OBJS                 = $(PARENT_PRG_ROOT)/obj/xcps/cpsv3000/xcps.o  \
                         $(OBJS_DIR)/agentdb/agentdb.o \
						 $(XCPS_SCTP_LIB_ROOT)/libsctpx86_64.a \
						 $(OBJS_DIR)/agenttrace/agenttrace.o \
						 $(OBJS_DIR)/agentoam/agentoam.o \
						 $(OBJS_DIR)/agentua/agentua.o \
                         $(OBJS_DIR)/mqtt/mqtt.o \
						 $(OBJS_DIR)/agenthlr/agenthlr.o \
						 $(OBJS_DIR)/$(BUILD_MODULE_NAME)/$(BUILD_MODULE_NAME).o
  MAKES_LIST           +=$(SRC_ROOT)/mqtt/makefile \
                         $(SRC_ROOT)/agenttrace/makefile   \
						 $(SRC_ROOT)/agentoam/makefile   \
						 $(SRC_ROOT)/hlragent/makefile \
						 $(SRC_ROOT)/agentua/makefile \
						 $(SRC_ROOT)/agenthlr/makefile \
                         $(SRC_ROOT)/agentdb/makefile 
                        
                        
export GLOBAL_MACRO +=
                  
########################################################################
# Set oam make file
########################################################################

ifeq ($(CPU),x86_64)
	OBJS += $(XCPS_SCTP_LIB_ROOT)/libsctpx86_64.a
else
	OBJS += $(XCPS_SCTP_LIB_ROOT)/libsctp32.a 
endif
 
  include $(RULES_ROOT)/make_link.rule
endif
                  
ifeq ($(MAKE_PLATFORM), WIN32)
  LIB_OBJS  = 
              
else
  LIB_OBJS  = -L$(ACE_LIB_ROOT) -lACEXML -lACEXML_Parser -lACE \
			  -L$(AGENTHARDDB_LIB_ROOT) -lagentharddb  \
			  -L$(MQTT_LIB_ROOT) -lmqtt  \
			  -L$(AGENTDB_LIB_ROOT) -lagentdb  \
			  -L$(AGENTOAM_LIB_ROOT) -lagentoam  \
			  -L$(AGENTUA_LIB_ROOT) -lagentua  \
			  -L$(MYSQL_LIB_ROOT) -lmysqlclient \
			  -L$(MYSQLDBDLL_LIB_ROOT) -lmysqldbdll  
#              -L$(ORACLE_LIB_ROOT) -L$(dir ${ORACLE_LIB}) -locci -lclntsh \ 
#              -L$(DBDLL_LIB_ROOT) -ldbproj  		  
              			  
               

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

#包含源类型的规则文件
ifeq ($(MAKE_TYPE), SRC_TYPE)
  include $(RULES_ROOT)/make_src.rule
endif

