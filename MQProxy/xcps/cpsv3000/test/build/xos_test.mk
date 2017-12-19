########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################

#�û����Խ����ӵ�ͷ�ļ�/Դ�ļ�Ŀ¼���������ڵ������ļ��У��ڴ˰�����
#Ȼ���ں����������еı���
include $(XCPS_ROOT)/app_common/xcps.dir
include $(XCPS_ROOT)/cpsv3000/xos.dir

MAKE_TYPE  = LINK_TYPE
MAKEFILE_NAME=$(XOSTEST_ROOT)/build/xos_test.mk
#���屾makefile�����Ŀ���ļ���������hi.o
ifeq ($(MAKE_PLATFORM), VXWORKS)
	PRODUCT_NAME	      = $(XOSTEST_ROOT)/bin/xostest.o
else
	PRODUCT_NAME	      = $(XOSTEST_ROOT)/bin/xostest
endif


OBJS =  $(subst $(subst \,/,$(SRC_ROOT)),$(subst \,/,$(OBJS_DIR)),$(subst \,/,$(TEST_SRC_ROOT)))/test.o
OBJS += $(subst $(subst \,/,$(SRC_ROOT)),$(subst \,/,$(OBJS_DIR)),$(subst \,/,$(XOS_PORT)))/xos_op.o
	
MAKES_LIST = $(TEST_SRC_ROOT)/test.mk
MAKES_LIST += $(XOS_PORT)/xos_op.mak

				
ifeq ($(MAKE_PLATFORM), LINUX)
#����ģ���Ժ�����Ҫ�����ӵĿ⣬�������������
	LIB_OBJS        = 
#	LIB_OBJS        += -lpthread -ldl -lc -lbfd -pg
endif	

#������Ҫ���ӵĿ��ļ�
include $(RULES_ROOT)/makefile.rules

include $(RULES_ROOT)/make_link.rule
