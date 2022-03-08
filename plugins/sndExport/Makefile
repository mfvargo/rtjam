# Stuff for all
SRC_DIRS ?= ./src ../common
CC = gcc
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

#targets
SNDEXPORT ?= ../../bin/sndExport
SNDPLAY ?= ../../bin/sndPlay
SNDSPLIT ?= ../../bin/sndSplit
TARGETS ?= $(SNDEXPORT) $(SNDPLAY) $(SNDSPLIT)

SNDEXPORTSRCS = \
	./src/sndExport.cpp \

SNDPLAYSRCS = \
	./src/sndPlay.cpp \

SNDSPLITSRCS = \
	./src/sndSplit.cpp \

COMMON = \
	../common/JamNetStuff.cpp \
	../common/TimersAndStats.cpp \
	../common/JamMixer.cpp \
	../common/JitterBuffer.cpp \
	../common/ChannelMap.cpp \
	../common/ReplayStream.cpp \

COMMONOBJS := $(addsuffix .o,$(basename $(COMMON)))
SNDEXPORTOBJS := $(addsuffix .o,$(basename $(SNDEXPORTSRCS))) 
SNDPLAYOBJS := $(addsuffix .o,$(basename $(SNDPLAYSRCS))) 
SNDSPLITOBJS := $(addsuffix .o,$(basename $(SNDSPLITSRCS))) 
DEPS := $(SNDPLAYOBJS:.o=.d) $(SNDEXPORTOBJS:.o=.d) $(SNDSPLITOBJS:.o=.d) $(COMMONOBJS:.o=.d) 

LDLIBS1 := -Bdynamic -lm -lstdc++ -lpthread -lsndfile -lstdc++fs
LDLIBS2 := -Bdynamic -lm -lstdc++ -lpthread -ljack -lrt 

CPPFLAGS ?= -std=c++17 $(INC_FLAGS) -MMD -MP -g -Wno-psabi

all:	$(TARGETS)

$(SNDEXPORT): $(SNDEXPORTOBJS) $(COMMONOBJS)
	$(CC) $(LDFLAGS) $(SNDEXPORTOBJS) $(COMMONOBJS) -o $@ $(LOADLIBES) $(LDLIBS1)

$(SNDPLAY): $(SNDPLAYOBJS) $(COMMONOBJS)
	$(CC) $(LDFLAGS) $(SNDPLAYOBJS) $(COMMONOBJS) -o $@ $(LOADLIBES) $(LDLIBS2)

$(SNDSPLIT): $(SNDSPLITOBJS) $(COMMONOBJS)
	$(CC) $(LDFLAGS) $(SNDSPLITOBJS) $(COMMONOBJS) -o $@ $(LOADLIBES) $(LDLIBS1)


.PHONY: clean
clean:
	$(RM) $(SNDEXPORT) $(SNDEXPORTOBJS) $(SNDPLAYOBJS) $(SNDSPLITOBJS) $(COMMONOBJS) $(DEPS)

-include $(DEPS)