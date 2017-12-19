########################################################################
#      Makefile file for                                               #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################

#��������makefile�ļ�������include $(SRC_ROOT)/protocol/h248/mgco.dir
#�û����Խ����ӵ�ͷ�ļ�/Դ�ļ�Ŀ¼���������ڵ������ļ��У��ڴ˰�����
#Ȼ���ں����������еı���
include $(V3000_ROOT)/xos.dir

# ���ñ�makefile�ļ������ͣ�ֵ����SRC_TYPE �� LINK_TYPE
MAKE_TYPE             = SRC_TYPE

#�Ƿ���Ҫ֧��Trillium SSI�Ĺ���ֵΪYES��ʾ��Ҫ��
NEED_TRI_SSI	      = 

#���屾makefile�����Ŀ���ļ���������hi.o
PRODUCT_NAME	      = prt.o

#���屾makefile��·���������ڼ���·��������$(SRC_ROOT)/protocol/tucl/bdy/makefile
MAKEFILE_NAME         = $(V3000_ROOT)/makefile.mak

ifeq ($(MAKE_TYPE), SRC_TYPE)
#����Դ���͵�makefile

#����Դ�ļ���·��������$(SRC_ROOT)/protocol/tucl/bdy/src
  VPATH               = $(XOS_PORT)
  
#����Դ�ļ��б��û�����ָ������ͨ��ȱʡ��ʽ�ҳ�Դ·���µ�����Դ�ļ���.c/.cpp/.s����  
  SRCS                = $(wildcard $(foreach EVERY_VPATH,$(VPATH),$(EVERY_VPATH)/*.c $(EVERY_VPATH)/*.cpp $(EVERY_VPATH)/*.s))

#�����û��ض���ͷ�ļ�����·������ʽ��-I·����������-I$(SRC_ROOT)/protocol/tucl/bdy/inc
#����ܶ࣬�����û��з��š�\������
  EXTRA_INCLUDE	    = $(XOS_INC)
  										

#�����û��ض��ı���ѡ���ʽ��-D����꣬����-DGCP_ASN��
  EXTRA_DEFINE      =

else
#�����������͵�makefile

#����Ŀ���ļ���·����������$(subst $(SRC_ROOT),$(OBJS_DIR),$(MAKEFILE_DIR))/bdy/hi.o
#$(subst $(SRC_ROOT),$(OBJS_DIR),$(MAKEFILE_DIR))���ڼ����Ŀ���ļ��Ĵ��·��
#����ͨ����\�����з����Ӷ��·����
#����ͨ����OBJS += ����ʽ����·������
OBJS             = 

#�����Ӧ��makefile�ļ���·����������$(MAKEFILE_DIR)/bdy/makefile
#ͨ����makefile�ļ����Դ���OBJS�ж����Ŀ���ļ���
MAKES_LIST       = 


OBJS       += $(subst $(SRC_ROOT),$(OBJS_DIR),$(MAKEFILE_DIR))/xosxml/xosxml.o
MAKES_LIST += $(XOS_XML)/makefile

#������Ҫ���ӵĿ��ļ�·��
LIB_PATH            =

#�����������͵Ĺ����ļ�
include $(RULES_ROOT)/make_link.rule

endif

#������Ҫ���ӵĿ��ļ�
LIB_OBJS              = 


########################################################################
# Check common definitions and include rules  
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

