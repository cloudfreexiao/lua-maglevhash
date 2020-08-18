SKYNET_ROOT ?= ../../skynet
include $(SKYNET_ROOT)/platform.mk

PLAT ?= none

TARGET =  ../../luaclib/maglevhash.so

ifeq ($(PLAT), macosx)
	CFLAGS = -g -O2 -dynamiclib -Wl,-undefined,dynamic_lookup
else
ifeq ($(PLAT), linux)
	CFLAGS = -g -O2 -shared -fPIC
endif
endif

LUA_LIB ?= $(SKYNET_ROOT)/3rd/lua/
LUA_INC ?= $(SKYNET_ROOT)/3rd/lua/
SKYNET_SRC ?= $(SKYNET_ROOT)/skynet-src

SRC = .

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(foreach dir, $(SRC), $(wildcard $(dir)/*.c))
	$(CC) $(CFLAGS) $(SHARED) -I$(LUA_INC) -I$(SKYNET_SRC) $^ -o $@

clean:
	rm -f *.o $(TARGET)