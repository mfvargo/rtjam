TARGET ?= ../../bin/sndExport
SRC_DIRS ?= ./src ../common

CC = gcc

#SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
SRCS = \
	./src/main.cpp \
	../common/JamNetStuff.cpp \
	../common/TimersAndStats.cpp \
	../common/JamMixer.cpp \
	../common/JitterBuffer.cpp \
	../common/ChannelMap.cpp \
	../common/ReplayStream.cpp \


OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LDLIBS := -Bdynamic -lm -lstdc++ -lpthread -lsndfile

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g -Wno-psabi

all:	$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)