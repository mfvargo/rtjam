TARGET ?= ../../bin/rtjam-sound
SRC_DIRS ?= ./src ../common

CC = gcc

#SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
SRCS = \
	./src/main.cpp \
	./src/PluginRTJam.cpp \
	../common/JamNetStuff.cpp \
	../common/TimersAndStats.cpp \
	../common/JamMixer.cpp \
	../common/JitterBuffer.cpp \
	../common/ChannelMap.cpp \
	../common/SharedMemory.cpp \
	../common/MessageQueue.cpp \
	../common/LevelData.cpp \
	../common/ParamData.cpp \
	../common/LightData.cpp \


OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LDLIBS := -lm -lstdc++ -lpthread -ljack -lrt

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g -Wno-psabi

all:	$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)