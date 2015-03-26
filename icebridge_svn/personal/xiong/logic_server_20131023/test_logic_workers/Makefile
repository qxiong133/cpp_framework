###################################################################
# Makefile template
# author hitzheng@gmail.com
# date 2012-03-18
# usage:
#   1. customize the USER-DEFINED SECTION
#   2. make         build the target
#   3. make clean   clean all
#   4. make objs    build .o and .d only
#   5. make help    show help
#   6. make show    show build variables
#   7. add something to BEFOREBUILD or AFTERBUILD if needed
###################################################################

#### USER-DEFINED SECTION #########################################
# compiler
CC := g++

# target name
TARGET := good.exe

# compile flags
#CFLAGS := -g -Wall -I/home/xiong/poco/include -I./include -I.
CFLAGS := -g -Wall -I. -I../include/ -I/home/xiong/opt/libuv/include -I/home/xiong/opt/redis/include -I/home/xiong/opt/zeromq/include -I/home/xiong/opt/poco/include -I/home/xiong/opt/czmq/include -I/home/xiong/opt/boost/include



# compile flags
#CPPFLAGS :=  -I/home/xiong/poco/include -I./include
CPPFLAGS := -g -Wall -I. -I../include/ -I/home/xiong/opt/libuv/include -I/home/xiong/opt/redis/include -I/home/xiong/opt/zeromq/include -I/home/xiong/opt/poco/include -I/home/xiong/opt/czmq/include -I/home/xiong/opt/boost/include


# ld flags for libraries
#LDFLAGS := -L/home/xiong/poco/lib -lPocoFoundation  -lPocoNet  -lPocoUtil  -lPocoXML
#LDFLAGS := -L/usr/lib/x86_64-linux-gnu/ -L/home/xiong/opt/libuv/lib -L/home/xiong/opt/redis/lib -L/home/xiong/opt/zeromq/lib -L/home/xiong/opt/poco/lib -L/home/xiong/opt/czmq/lib -lczmq -lzmq -lPocoFoundation  -lPocoNet  -lPocoUtil  -lPocoXML -luv -lpthread -lrt -luuid -static
LDFLAGS := -L/usr/lib/x86_64-linux-gnu/ -L/home/xiong/opt/libuv/lib -L/home/xiong/opt/redis/lib -L/home/xiong/opt/zeromq/lib -L/home/xiong/opt/poco/lib -L/home/xiong/opt/czmq/lib -lczmq -lzmq -lPocoFoundation -lPocoNet  -lPocoUtil  -lPocoXML -luv -lpthread -lrt -luuid 

# define 32bit or 64 bit
PLATFORM    = $(strip $(shell echo `uname -m`))

ifneq ($(PLATFORM),x86_64)
	MFLAGS := 32
else
	MFLAGS := 64
endif

MFLAGS      := $(strip $(MFLAGS))


# target type: exe static shared
TYPE := exe

# source file extension
SRCEXT := .c .cpp .cc

# output directory
OBJ_DIR := .

# dirs exclude from searching sources
EXCLUDE_DIRS := $(OBJ_DIR) .svn include

# files exclude from source list
EXCLUDE_FILES :=

#### DO NOT CHANGE THIS SECTION ###################################
EMPTY :=
SPACE := $(EMPTY) $(EMPTY)

TARGET := $(strip $(TARGET))
TYPE := $(strip $(TYPE))

ifeq ($(TARGET),)
  TARGET := a.out
endif
TARGET := $(TARGET:%=$(OBJ_DIR)/%)

ifeq ($(filter $(TYPE),exe static shared),)
$(error type $TYPE is error)
endif

# regexp for exclude dirs used in grep
EXCPAT := $(foreach d,$(EXCLUDE_DIRS),\\/$(d)\\\>)
EXCPAT := $(subst $(SPACE),\\\|,$(EXCPAT))
#EXCPAT := "svn"

# get all dirs exclude the EXCLUDE_DIRS
SRCDIRS := $(patsubst ./%,%,$(shell find . -type d | grep -v $(EXCPAT)))

# get the source list
SRCLIST := $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*, $(SRCEXT))))
SRCLIST := $(filter-out $(EXCLUDE_FILES), $(SRCLIST))

# objects list
#OBJS := $(basename $(notdir $(SRCLIST)))
#OBJS := $(OBJS:%=$(OBJ_DIR)/%.o)
OBJS := $(patsubst %.cpp,%.o,$(SRCLIST))
OBJS := $(patsubst %.c,%.o,$(OBJS))
OBJS := $(patsubst %.cc,%.o,$(OBJS))

DEPS := $(OBJS:.o=.d)

######################################################################
%.d: %.cpp
	@echo "update $@ ..."; \
	echo -n $< | sed s/\.cpp/\.o:/ > $@; \
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $< | sed '1s/.*.://' >> $@;


%.d: %.cc
	@echo "update $@ ..."; \
	echo -n $< | sed s/\.cc/\.o:/ > $@; \
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $< | sed '1s/.*.://' >> $@;

%.d: %.c
	@echo "update $@ ..."; \
	echo -n $< | sed s/\.c/\.o:/ > $@; \
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $< | sed '1s/.*.://' >> $@;

###################################################################################
.PHONY: all objs clean show help

all : BEFOREBUILD $(TARGET) AFTERBUILD

ifneq ($(DEPS),)
-include $(DEPS)
endif

objs : $(OBJS)

# TARGET RULE
$(TARGET) : $(OBJS)
ifeq ($(TYPE),exe)
	@echo "build exe...."
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

else ifeq ($(TYPE),static)
	@echo "build static library..."
	ar rcs $@ $(OBJS)

else ifeq ($(TYPE),shared)
	@echo "build shared objects..."
	$(CC) -fpic -shared $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

else
	@echo "target type:$(TYPE) error!!!"
endif

# DO SOMETHING HERE BEFORE BUILD
BEFOREBUILD:
	@echo "do preparations before build..."
	@if [ ! -e $(OBJ_DIR) ];then mkdir -p $(OBJ_DIR);echo "mkdir $(OBJ_DIR)";fi

# DO SOMETHING HERE AFTER BUILD
AFTERBUILD:
	@echo "build finish, do something here"

clean:
	$(RM) $(OBJS) $(DEPS) $(TARGET)


help:
	@echo "usage:"
	@echo "1. customize the USER-DEFINED SECTION"
	@echo "2. make  build the target"
	@echo "3. make clean    clean all"
	@echo "4. make objs build .o and .d only"
	@echo "5. make help show help"
	@echo "6. make show show build variables"
	@echo "7. add something BEFOREBUILD or AFTERBUILD if needed"

# DEBUG USE ONLY
show:
	@echo "TARGET: "$(TARGET)
	@echo "TYPE: "$(TYPE)
	@echo "CFLAGS: "$(CFLAGS)
	@echo "LDFLAGS: "$(LDFLAGS)
	@echo "EXCLUDE_DIRS: "$(EXCLUDE_DIRS)
	@echo "EXCLUDE_FILES: "$(EXCLUDE_FILES)
	@echo "EXCPAT: "$(EXCPAT)
	@echo "SRCDIRS: "$(SRCDIRS)
	@echo "SRCLIST: "$(SRCLIST)
	@echo "OBJS: "$(OBJS)
	@echo "DEPS: "$(DEPS)
	@echo "test: "$(test)
