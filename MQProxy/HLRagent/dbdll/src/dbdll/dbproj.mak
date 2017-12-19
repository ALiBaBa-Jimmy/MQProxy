########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################


########################################################################
# Set makefile's type (SRC_TYPE or LINK_TYPE) (you must set it)
########################################################################
MAKE_TYPE           = LINK_TYPE
NEED_TRI_SSI	      = YES
ifeq ($(MAKE_PLATFORM), WIN32)
  PRODUCT_NAME       = dbprojall.o
else
  ifeq ($(MAKE_PLATFORM), LINUX)
    PRODUCT_NAME       = $(DBPROJ_OUTPUT)
  endif
endif
MAKEFILE_NAME       = $(SRC_ROOT)/dbdll/dbproj.mak


ifeq ($(MAKE_TYPE), SRC_TYPE)
  VPATH               = 
  SRCS                = $(wildcard $(foreach EVERY_VPATH,$(VPATH),$(EVERY_VPATH)/*.c $(EVERY_VPATH)/*.cpp $(EVERY_VPATH)/*.s))
  EXTRA_INCLUDE	      = 
  EXTRA_DEFINE        = 
else
  LIB_PATH            = 	
	OBJS		     = $(OBJS_DIR)/dbdll/dbdll.o
	
	MAKES_LIST       += $(SRC_ROOT)/dbdll/makefile
	                 
export GLOBAL_MACRO +=
include $(RULES_ROOT)/make_link.rule

ifndef ORACLE_LIB
exit_to_setenv:
	@echo;
	@echo ERROR: ORACLE_LIB including it's directory and file name must be set in setevent.bat!
	@echo;
endif
  

ifeq ($(MAKE_PLATFORM), WIN32)
  LIB_OBJS  = $(ORACLE_LIB)
              
else
  LIB_OBJS  = -L$(dir ${ORACLE_LIB})  -L$(PRG_ROOT)/bin/linux -L$(OCI_LIB_ROOT)  -L$(ACE_LIB_ROOT) -lACE -locci
endif

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
# targets definition
########################################################################

