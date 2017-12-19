########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################
#��������makefile�ļ�������include $(SRC_ROOT)/protocol/h248/mgco.dir
#�û����Խ����ӵ�ͷ�ļ�/Դ�ļ�Ŀ¼���������ڵ������ļ��У��ڴ˰�����
#Ȼ���ں����������еı���
include $(XCPS_ROOT)/app_common/xcps.dir

#���ñ�makefile�ļ������ͣ�ֵ���� SRC_TYPE �� LINK_TYPE
MAKE_TYPE             = LINK_TYPE

MAKEFILE_NAME         = $(V3000_ROOT)/makecps.mak
PRODUCT_NAME          = xcps.o

OBJS  =  $(OBJS_DIR)/xcps/cpsv3000/port/xos_op.o

ifeq ($(NEED_OAM), 1)
	 OBJS += $(OBJS_DIR)/xcps/app_common/oam/oam.o
endif

ifeq ($(NEED_CPPFRM), 1)
	 OBJS += $(OBJS_DIR)/xcps/app_common/cppfrm/cppfrm.o
endif

MAKES_LIST  = $(V3000_ROOT)/port/xos_op.mak

ifeq ($(NEED_OAM), 1)
 MAKES_LIST += $(APP_COMMON)/oam/makefile
endif

ifeq ($(NEED_CPPFRM), 1)
 MAKES_LIST += $(APP_COMMON)/cppfrm/makefile
endif

LIB_PATH     =

#�����������͵Ĺ����ļ�
include $(RULES_ROOT)/make_link.rule


########################################################################
# Check definitions and include rules  
########################################################################
#���Դ�ļ��ĸ�·���Ƿ���
ifndef SRC_ROOT
exit_to_setenv:
    @echo Must define SRC_ROOT!
else
#���Ŀ���ļ��ĸ�·���Ƿ���
ifndef OBJS_DIR
exit_to_setenv:
    @echo Must define OBJS_DIR!
else
#���CPU�Ƿ���
ifndef CPU
exit_to_setenv:
    @echo Must define CPU!
else
#����makefile��ƽ̨�����ļ���
   include $(RULES_ROOT)/makefile.rules
endif
endif
endif

########################################################################
# targets definition
########################################################################

#����Դ���͵Ĺ����ļ�
ifeq ($(MAKE_TYPE), SRC_TYPE)
  include $(RULES_ROOT)/make_src.rule
endif
