build.bat
ss.win32.mak
  OBJS = $(OBJS_DIR)/xcps/app_common/xcps.$(OBJTYPE) \
         $(OBJS_DIR)/xcps/app_common/saap/saapall.o \
         $(SXC_OBJ_DIR)/blib/blib.o \

  MAKES_LIST = $(XCPS_ROOT)/app_common/makefile   \
  		       $(XCPS_ROOT)/app_common/saap/makefile \

