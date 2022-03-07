GIT_HASH=`git rev-parse HEAD`
COMPILE_TIME=`date -u +'%Y-%m-%d %H:%M:%S UTC'`
GIT_BRANCH=`git branch | grep "^\*" | sed 's/^..//'`
export VERSION_FLAGS=-DGIT_HASH="\"$(GIT_HASH)\"" -DCOMPILE_TIME="\"$(COMPILE_TIME)\"" -DGIT_BRANCH="\"$(GIT_BRANCH)\""


TARGET ?= ../../bin/rtjam-box
SRC_DIRS ?= ./src ../common

CC = gcc

#SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
SRCS = \
	./src/main.cpp \
	../common/easywsclient.cpp \
	../common/MessageQueue.cpp \
	../common/SharedMemory.cpp \
	../common/RTJamNationApi.cpp \
	../common/Settings.cpp \
	../common/LightData.cpp \
	../common/TimersAndStats.cpp \

OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LDLIBS := -lm -lstdc++ -lpthread -ljack -lrt -lcurl -lfastcgipp

CPPFLAGS ?= -std=c++17 $(INC_FLAGS) $(VERSION_FLAGS) -MMD -MP -g -Wno-psabi

all:	$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
