########################################################################
#      makefile for PRODUCT                                            #
#      Created by                                                      #
#      Created time:                                                   #
########################################################################


########################################################################
# Set makefile's type (SRC_TYPE or LINK_TYPE) (you must set it)
########################################################################
DIR_PRODUCT_RULE = $(SRC_ROOT)/build/prod_rule

MAKE_TYPE           = LINK_TYPE
MAKEFILE_NAME       =$(DIR_PRODUCT_RULE)/makefile
########################################################################
# Check for product
# 
########################################################################
PRODUCT_LIST        = ISUN_SS ISUN_HLR ISUN_EBSC

#检查产品是否支持
ifeq "$(findstring $(PRODUCT),$(PRODUCT_LIST))" ""
exit_to_setenv:
	@echo You must set PRODUCT firstly!
	@echo The PRODUCT variables should have one of the following values:
	@echo $(PRODUCT_LIST)
endif
#针对不同的产品定义配置文件。
#配置文件用于定义产品宏、产品组件的目标名、产品组件目标的makefile列表和子目标文件列表。
ifeq ($(PRODUCT), ISUN_SS)
     include $(DIR_PRODUCT_RULE)/ss.cfg
endif

###########################################################################
#   Define all module name & makefile lists                               #
###########################################################################
MAKES_LIST  = $(MAKES_LIST1)  $(MAKES_LIST2)  $(MAKES_LIST3)  $(MAKES_LIST4)  \
              $(MAKES_LIST5)  $(MAKES_LIST6)  $(MAKES_LIST7)  $(MAKES_LIST8)  \
              $(MAKES_LIST9)  $(MAKES_LIST10) $(MAKES_LIST11) $(MAKES_LIST12) \
              $(MAKES_LIST13) $(MAKES_LIST14) $(MAKES_LIST15) $(MAKES_LIST16) \
              $(MAKES_LIST17) $(MAKES_LIST18) $(MAKES_LIST19) $(MAKES_LIST20) \
              $(MAKES_LIST21) $(MAKES_LIST22) $(MAKES_LIST23) $(MAKES_LIST24) \
              $(MAKES_LIST25) $(MAKES_LIST26) $(MAKES_LIST27) $(MAKES_LIST28) \
              $(MAKES_LIST29) $(MAKES_LIST30) $(MAKES_LIST31) $(MAKES_LIST32) \
              $(MAKES_LIST33) $(MAKES_LIST34) $(MAKES_LIST35) $(MAKES_LIST36) \
              $(MAKES_LIST37) $(MAKES_LIST38) $(MAKES_LIST39) $(MAKES_LIST40) \
              $(MAKES_LIST41) $(MAKES_LIST42) $(MAKES_LIST43) $(MAKES_LIST44) \
              $(MAKES_LIST45) $(MAKES_LIST46) $(MAKES_LIST47) $(MAKES_LIST48) \
              $(MAKES_LIST49) $(MAKES_LIST50) $(MAKES_LIST51) $(MAKES_LIST52) \
              $(MAKES_LIST53) $(MAKES_LIST54) $(MAKES_LIST55) $(MAKES_LIST56) \
              $(MAKES_LIST57) $(MAKES_LIST58) $(MAKES_LIST59) $(MAKES_LIST60) \
              $(MAKES_LIST61) $(MAKES_LIST62) $(MAKES_LIST63) $(MAKES_LIST64) \
              $(MAKES_LIST65) $(MAKES_LIST66) $(MAKES_LIST67) $(MAKES_LIST68) \
              $(MAKES_LIST69) $(MAKES_LIST70) $(MAKES_LIST71) $(MAKES_LIST72) \
              $(MAKES_LIST73) $(MAKES_LIST74) $(MAKES_LIST75) $(MAKES_LIST76) \
              $(MAKES_LIST77) $(MAKES_LIST78) $(MAKES_LIST79) $(MAKES_LIST80) \
              $(MAKES_LIST81) $(MAKES_LIST82) $(MAKES_LIST83) $(MAKES_LIST84) \
              $(MAKES_LIST85) $(MAKES_LIST86) $(MAKES_LIST87) $(MAKES_LIST88) \
              $(MAKES_LIST89) $(MAKES_LIST90) $(MAKES_LIST91) $(MAKES_LIST92) \
              $(MAKES_LIST93) $(MAKES_LIST94) $(MAKES_LIST95) $(MAKES_LIST96) \
              $(MAKES_LIST97) $(MAKES_LIST98) $(MAKES_LIST99) \
              $(MAKES_LIST100) $(MAKES_LIST101) $(MAKES_LIST102) $(MAKES_LIST103)\
              $(MAKES_LIST104) $(MAKES_LIST105) $(MAKES_LIST106) $(MAKES_LIST107)\
              $(MAKES_LIST108) $(MAKES_LIST109) \
              $(MAKES_LIST110) $(MAKES_LIST111) $(MAKES_LIST112) $(MAKES_LIST113)\
              $(MAKES_LIST114) $(MAKES_LIST115) $(MAKES_LIST116) $(MAKES_LIST117)\
              $(MAKES_LIST118) $(MAKES_LIST119) \
              $(MAKES_LIST120) $(MAKES_LIST121) $(MAKES_LIST122) $(MAKES_LIST123)\
              $(MAKES_LIST124) $(MAKES_LIST125) $(MAKES_LIST126) $(MAKES_LIST127)\
              $(MAKES_LIST128) $(MAKES_LIST129) \
              $(MAKES_LIST130) $(MAKES_LIST131) $(MAKES_LIST132) $(MAKES_LIST133)\
              $(MAKES_LIST134) $(MAKES_LIST135) $(MAKES_LIST136) $(MAKES_LIST137)\
              $(MAKES_LIST138) $(MAKES_LIST139) \
              $(MAKES_LIST140) $(MAKES_LIST141) $(MAKES_LIST142) $(MAKES_LIST143)\
              $(MAKES_LIST144) $(MAKES_LIST145) $(MAKES_LIST146) $(MAKES_LIST147)\
              $(MAKES_LIST148) $(MAKES_LIST149) \
              $(MAKES_LIST150) $(MAKES_LIST151) $(MAKES_LIST152) $(MAKES_LIST153)\
              $(MAKES_LIST154) $(MAKES_LIST155) $(MAKES_LIST156) $(MAKES_LIST157)\
              $(MAKES_LIST158) $(MAKES_LIST159) \
              $(MAKES_LIST160) $(MAKES_LIST161) $(MAKES_LIST162) $(MAKES_LIST163)\
              $(MAKES_LIST164) $(MAKES_LIST165) $(MAKES_LIST166) $(MAKES_LIST167)\
              $(MAKES_LIST168) $(MAKES_LIST169) \
              $(MAKES_LIST170) $(MAKES_LIST171) $(MAKES_LIST172) $(MAKES_LIST173)\
              $(MAKES_LIST174) $(MAKES_LIST175) $(MAKES_LIST176) $(MAKES_LIST177)\
              $(MAKES_LIST178) $(MAKES_LIST179) \
              $(MAKES_LIST180) $(MAKES_LIST181) $(MAKES_LIST182) $(MAKES_LIST183)\
              $(MAKES_LIST184) $(MAKES_LIST185) $(MAKES_LIST186) $(MAKES_LIST187)\
              $(MAKES_LIST188) $(MAKES_LIST189) \
              $(MAKES_LIST190) $(MAKES_LIST191) $(MAKES_LIST192) $(MAKES_LIST193)\
              $(MAKES_LIST194) $(MAKES_LIST195) $(MAKES_LIST196) $(MAKES_LIST197)\
              $(MAKES_LIST198) $(MAKES_LIST199) \
              $(MAKES_LIST200) $(MAKES_LIST201) $(MAKES_LIST202) $(MAKES_LIST203)\
              $(MAKES_LIST204) $(MAKES_LIST205) $(MAKES_LIST206) $(MAKES_LIST207)\
              $(MAKES_LIST208) $(MAKES_LIST209) \
              $(MAKES_LIST210) $(MAKES_LIST211) $(MAKES_LIST212) $(MAKES_LIST213)\
              $(MAKES_LIST214) $(MAKES_LIST215) $(MAKES_LIST216) $(MAKES_LIST217)\
              $(MAKES_LIST218) $(MAKES_LIST219) \
              $(MAKES_LIST220) $(MAKES_LIST221) $(MAKES_LIST222) $(MAKES_LIST223)\
              $(MAKES_LIST224) $(MAKES_LIST225) $(MAKES_LIST226) $(MAKES_LIST227)\
              $(MAKES_LIST228) $(MAKES_LIST229) \
              $(MAKES_LIST230) $(MAKES_LIST231) $(MAKES_LIST232) $(MAKES_LIST233)\
              $(MAKES_LIST234) $(MAKES_LIST235) $(MAKES_LIST236) $(MAKES_LIST237)\
              $(MAKES_LIST238) $(MAKES_LIST239) \
              $(MAKES_LIST240) $(MAKES_LIST241) $(MAKES_LIST242) $(MAKES_LIST243)\
              $(MAKES_LIST244) $(MAKES_LIST245) $(MAKES_LIST246) $(MAKES_LIST247)\
              $(MAKES_LIST248) $(MAKES_LIST249) \
              $(MAKES_LIST250) $(MAKES_LIST251) $(MAKES_LIST252) $(MAKES_LIST253)\
              $(MAKES_LIST254) $(MAKES_LIST255) $(MAKES_LIST256) $(MAKES_LIST257)\
              $(MAKES_LIST258) $(MAKES_LIST259) \
              $(MAKES_LIST260) $(MAKES_LIST261) $(MAKES_LIST262) $(MAKES_LIST263)\
              $(MAKES_LIST264) $(MAKES_LIST265) $(MAKES_LIST266) $(MAKES_LIST267)\
              $(MAKES_LIST268) $(MAKES_LIST269) \
              $(MAKES_LIST270) $(MAKES_LIST271) $(MAKES_LIST272) $(MAKES_LIST273)\
              $(MAKES_LIST274) $(MAKES_LIST275) $(MAKES_LIST276) $(MAKES_LIST277)\
              $(MAKES_LIST278) $(MAKES_LIST279) \
              $(MAKES_LIST280) $(MAKES_LIST281) $(MAKES_LIST282) $(MAKES_LIST283)\
              $(MAKES_LIST284) $(MAKES_LIST285) $(MAKES_LIST286) $(MAKES_LIST287)\
              $(MAKES_LIST288) $(MAKES_LIST289) \
              $(MAKES_LIST290) $(MAKES_LIST291) $(MAKES_LIST292) $(MAKES_LIST293)\
              $(MAKES_LIST294) $(MAKES_LIST295) $(MAKES_LIST296) $(MAKES_LIST297)\
              $(MAKES_LIST298) $(MAKES_LIST299) $(MAKES_LIST300)

              
ALL_PRODUCT = $(PROD1)        $(PROD2)        $(PROD3)        $(PROD4)        \
              $(PROD5)        $(PROD6)        $(PROD7)        $(PROD8)        \
              $(PROD9)        $(PROD10)       $(PROD11)       $(PROD12)       \
              $(PROD13)       $(PROD14)       $(PROD15)       $(PROD16)       \
              $(PROD17)       $(PROD18)       $(PROD19)       $(PROD20)       \
              $(PROD21)       $(PROD22)       $(PROD23)       $(PROD24)       \
              $(PROD25)       $(PROD26)       $(PROD27)       $(PROD28)       \
              $(PROD29)       $(PROD30)       $(PROD31)       $(PROD32)       \
              $(PROD33)       $(PROD34)       $(PROD35)       $(PROD36)       \
              $(PROD37)       $(PROD38)       $(PROD39)       $(PROD40)       \
              $(PROD41)       $(PROD42)       $(PROD43)       $(PROD44)       \
              $(PROD45)       $(PROD46)       $(PROD47)       $(PROD48)       \
              $(PROD49)       $(PROD50)       $(PROD51)       $(PROD52)       \
              $(PROD53)       $(PROD54)       $(PROD55)       $(PROD56)       \
              $(PROD57)       $(PROD58)       $(PROD59)       $(PROD60)       \
              $(PROD61)       $(PROD62)       $(PROD63)       $(PROD64)       \
              $(PROD65)       $(PROD66)       $(PROD67)       $(PROD68)       \
              $(PROD69)       $(PROD70)       $(PROD71)       $(PROD72)       \
              $(PROD73)       $(PROD74)       $(PROD75)       $(PROD76)       \
              $(PROD77)       $(PROD78)       $(PROD79)       $(PROD80)       \
              $(PROD81)       $(PROD82)       $(PROD83)       $(PROD84)       \
              $(PROD85)       $(PROD86)       $(PROD87)       $(PROD88)       \
              $(PROD89)       $(PROD90)       $(PROD91)       $(PROD92)       \
              $(PROD93)       $(PROD94)       $(PROD95)       $(PROD96)       \
              $(PROD97)       $(PROD98)       $(PROD99)       \
              $(PROD100)      $(PROD101)      $(PROD102)      $(PROD103)\
              $(PROD104)      $(PROD105)      $(PROD106)      $(PROD107)\
              $(PROD108)      $(PROD109)\
              $(PROD110)      $(PROD111)      $(PROD112)      $(PROD113)\
              $(PROD114)      $(PROD115)      $(PROD116)      $(PROD117)\
              $(PROD118)      $(PROD119)\
              $(PROD120)      $(PROD121)      $(PROD122)      $(PROD123)\
              $(PROD124)      $(PROD125)      $(PROD126)      $(PROD127)\
              $(PROD128)      $(PROD129)\
              $(PROD130)      $(PROD131)      $(PROD132)      $(PROD133)\
              $(PROD134)      $(PROD135)      $(PROD136)      $(PROD137)\
              $(PROD138)      $(PROD139)\
              $(PROD140)      $(PROD141)      $(PROD142)      $(PROD143)\
              $(PROD144)      $(PROD145)      $(PROD146)      $(PROD147)\
              $(PROD148)      $(PROD149)\
              $(PROD150)      $(PROD151)      $(PROD152)      $(PROD153)\
              $(PROD154)      $(PROD155)      $(PROD156)      $(PROD157)\
              $(PROD158)      $(PROD159)\
              $(PROD160)      $(PROD161)      $(PROD162)      $(PROD163)\
              $(PROD164)      $(PROD165)      $(PROD166)      $(PROD167)\
              $(PROD168)      $(PROD169)\
              $(PROD170)      $(PROD171)      $(PROD172)      $(PROD173)\
              $(PROD174)      $(PROD175)      $(PROD176)      $(PROD177)\
              $(PROD178)      $(PROD179)\
              $(PROD180)      $(PROD181)      $(PROD182)      $(PROD183)\
              $(PROD184)      $(PROD185)      $(PROD186)      $(PROD187)\
              $(PROD188)      $(PROD189)\
              $(PROD190)      $(PROD191)      $(PROD192)      $(PROD193)\
              $(PROD194)      $(PROD195)      $(PROD196)      $(PROD197)\
              $(PROD198)      $(PROD199)\
              $(PROD200)      $(PROD201)      $(PROD202)      $(PROD203)\
              $(PROD204)      $(PROD205)      $(PROD206)      $(PROD207)\
              $(PROD208)      $(PROD209)\
              $(PROD210)      $(PROD211)      $(PROD212)      $(PROD213)\
              $(PROD214)      $(PROD215)      $(PROD216)      $(PROD217)\
              $(PROD218)      $(PROD219)\
              $(PROD220)      $(PROD221)      $(PROD222)      $(PROD223)\
              $(PROD224)      $(PROD225)      $(PROD226)      $(PROD227)\
              $(PROD228)      $(PROD229)\
              $(PROD230)      $(PROD231)      $(PROD232)      $(PROD233)\
              $(PROD234)      $(PROD235)      $(PROD236)      $(PROD237)\
              $(PROD238)      $(PROD239)\
              $(PROD240)      $(PROD241)      $(PROD242)      $(PROD243)\
              $(PROD244)      $(PROD245)      $(PROD246)      $(PROD247)\
              $(PROD248)      $(PROD249)\
              $(PROD250)      $(PROD251)      $(PROD252)      $(PROD253)\
              $(PROD254)      $(PROD255)      $(PROD256)      $(PROD257)\
              $(PROD258)      $(PROD259)\
              $(PROD260)      $(PROD261)      $(PROD262)      $(PROD263)\
              $(PROD264)      $(PROD265)      $(PROD266)      $(PROD267)\
              $(PROD268)      $(PROD269)\
              $(PROD270)      $(PROD271)      $(PROD272)      $(PROD273)\
              $(PROD274)      $(PROD275)      $(PROD276)      $(PROD277)\
              $(PROD278)      $(PROD279)\
              $(PROD280)      $(PROD281)      $(PROD282)      $(PROD283)\
              $(PROD284)      $(PROD285)      $(PROD286)      $(PROD287)\
              $(PROD288)      $(PROD289)\
              $(PROD290)      $(PROD291)      $(PROD292)      $(PROD293)\
              $(PROD294)      $(PROD295)      $(PROD296)      $(PROD297)\
              $(PROD298)      $(PROD299)      $(PROD300)
              

########################################################################
# include rules and common directory definition
########################################################################
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
#包含模块的定义
   include $(DIR_PRODUCT_RULE)/modules.def
#包含基本规则,包括操作系统规则、SSI规则.   
   include $(SRC_ROOT)/build/makefile.rules
endif
endif
endif

#提供第一个目标default
include $(DIR_PRODUCT_RULE)/link.rule

#-----------------------------------------------------------------------
