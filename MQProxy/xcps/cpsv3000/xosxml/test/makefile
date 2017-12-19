CC=gcc
LD=gcc

CPSV_HOME=$(PWD)/../..
XOSXML_HOME=$(CPSV_HOME)/xosxml

CFLAGS =-I$(XOSXML_HOME)/inc -I$(CPSV_HOME) -DXOS_LINUX
LDFLAGS = $(CPSV_HOME)/obj/linux/cpsv3000/xos.a

SRC=$(wildcard ./*.c)
OBJ=$(foreach n,$(SRC),./$(notdir $(subst .c,.o,$(n))))
TARGET=$(foreach n,$(SRC),./$(notdir $(subst .c,,$(n))))

CFLAGS += -I/usr/include
LDFLAGS += -L/usr/lib -lpthread -lc -lm -ldl -lbfd -lrt -lstdc++

all : $(TARGET)

runsuite:runsuite.o
	$(LD) -o $@ $< $(LDFLAGS)
test_xwriter:test_xwriter.o
	$(LD) -o $@ $< $(LDFLAGS)
runtest:runtest.o
	$(LD) -o $@ $< $(LDFLAGS)
runxmlconf:runxmlconf.o
	$(LD) -o $@ $< $(LDFLAGS)
testapi:testapi.o
	$(LD) -o $@ $< $(LDFLAGS)
testAutomata:testAutomata.o
	$(LD) -o $@ $< $(LDFLAGS)
testC14:testC14.o
	$(LD) -o $@ $< $(LDFLAGS)
testchar:testchar.o
	$(LD) -o $@ $< $(LDFLAGS)
testdict:testdict.o
	$(LD) -o $@ $< $(LDFLAGS)
testHTML:testHTML.o
	$(LD) -o $@ $< $(LDFLAGS)
testModule:testModule.o
	$(LD) -o $@ $< $(LDFLAGS)
testReader:testReader.o
	$(LD) -o $@ $< $(LDFLAGS)
testrecurse:testrecurse.o
	$(LD) -o $@ $< $(LDFLAGS)
testRegexp:testRegexp.o
	$(LD) -o $@ $< $(LDFLAGS)

testRelax:testRelax.o
	$(LD) -o $@ $< $(LDFLAGS)

testSAX:testSAX.o
	$(LD) -o $@ $< $(LDFLAGS)
testSchemas:testSchemas.o
	$(LD) -o $@ $< $(LDFLAGS)
testThreads:testThreads.o
	$(LD) -o $@ $< $(LDFLAGS)
testThreadsWin32:testThreadsWin32.o
	$(LD) -o $@ $< $(LDFLAGS)
testURI:testURI.o
	$(LD) -o $@ $< $(LDFLAGS)
testXPath:testXPath.o
	$(LD) -o $@ $< $(LDFLAGS)
xmlcateglog:xmlcateglog.o
	$(LD) -o $@ $< $(LDFLAGS)
xmllint:xmllint.o
	$(LD) -o $@ $< $(LDFLAGS)

readUnicode:readUnicode.o
	$(LD) -o $@ $< $(LDFLAGS)

$(OBJ):$(SRC)
	$(CC) -c -o $@ ./$(notdir $*).c $(CFLAGS)

clean:
	$(RM) -vf $(TARGET)
	$(RM) -vf $(OBJ)
