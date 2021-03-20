TARGET ?= ../../bin/leverReader
SRC_DIRS ?= ./src ../common

CC = gcc

#SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
SRCS = \
	./src/levelReader.cpp \
	../common/LevelData.cpp \
	../common/ParamData.cpp \
	../common/MessageQueue.cpp \
	../common/SharedMemory.cpp \
	../common/RTJamNationApi.cpp \

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