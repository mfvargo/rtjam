TARGET ?= ../../bin/uxfree
SRC_DIRS ?= ./src ../common

CC = gcc

#SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
SRCS = \
	./src/uxfree.cpp \
	./src/PluginRTJam.cpp \
	../common/LevelData.cpp \
	../common/ParamData.cpp \
	../common/JamNetStuff.cpp \
	../common/JamMixer.cpp \
	../common/JitterBuffer.cpp \
	../common/ChannelMap.cpp \
	../common/SharedMemory.cpp \
	../common/MessageQueue.cpp \
	../common/Settings.cpp \
	../common/JamDirectory.cpp \
	../common/TimersAndStats.cpp

OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LDLIBS := -lm -lstdc++ -lpthread -ljack -lrt -lcurl

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g

all:	$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)