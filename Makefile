SRC ?= src
BUILD ?= build
TESTS ?= tests
UNAME := $(shell uname)

AR ?= ar
CPPFLAGS = -std=c++11 -lpthread

ifeq ($(APP_DEBUG), true)
	CPPFLAGS += -g -O0
else
	CPPFLAGS += -O2
endif

PREFIX ?= /usr/local

SRCS += $(wildcard $(SRC)/*.cpp)

OBJS += $(SRCS:.cpp=.o)

all: build

%.o: %.cpp
	$(CC) $< $(CPPFLAGS) -c -o $@

build: $(BUILD)/lib/libchan.a
	mkdir -p $(BUILD)/include/chan
	cp -f $(SRC)/chan.h $(BUILD)/include/chan/chan.h

$(BUILD)/lib/libchan.a: $(OBJS)
	mkdir -p $(BUILD)/lib
	$(AR) -crs $@ $^

clean:
	rm -rf *.o $(BUILD) $(SRC)/*.o test

test: build
	mkdir -p $(BUILD)/tests
	g++ $(CPPFLAGS) -I$(build)/include -o $(BUILD)/tests/buffered $(TESTS)/buffered.cpp -Lbuild/lib -lchan
	#$(CC) $(CPPFLAGS) -I$(build)/include -o $(BUILD)/examples/unbuffered $(EXAMPLES)/unbuffered.c -Lbuild/lib -lchan -pthread
	#$(CC) $(CPPFLAGS) -I$(build)/include -o $(BUILD)/examples/close $(EXAMPLES)/close.c -Lbuild/lib -lchan -pthread
	#$(CC) $(CPPFLAGS) -I$(build)/include -o $(BUILD)/examples/select $(EXAMPLES)/select.c -Lbuild/lib -lchan -pthread

install: all
	mkdir -p $(PREFIX)/include/chan
	mkdir -p $(PREFIX)/lib
	cp -f $(SRC)/chan.h $(PREFIX)/include/chan/chan.h
	cp -f $(BUILD)/lib/libchan.a $(PREFIX)/lib/libchan.a

uninstall:
	rm -rf $(PREFIX)/include/chan/chan.h
	rm -rf $(PREFIX)/lib/libchan.a

.PHONY: build check test clean install uninstall
