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

build:
	mkdir -p $(BUILD)/include/chan
	cp -f $(SRC)/chan.h $(BUILD)/include/chan/chan.h

clean:
	rm -rf *.o $(BUILD) $(SRC)/*.o test

test: build
	mkdir -p $(BUILD)/tests
	g++ $(CPPFLAGS) -I$(build)/include -o $(BUILD)/tests/buffered $(TESTS)/buffered.cpp -pthread
	g++ $(CPPFLAGS) -I$(build)/include -o $(BUILD)/tests/unbuffered $(TESTS)/unbuffered.cpp -pthread
	g++ $(CPPFLAGS) -I$(build)/include -o $(BUILD)/tests/close $(TESTS)/close.cpp -pthread
	g++ $(CPPFLAGS) -I$(build)/include -o $(BUILD)/tests/select $(TESTS)/select.cpp -pthread
	g++ $(CPPFLAGS) -I$(build)/include -o $(BUILD)/tests/rolling $(TESTS)/rolling.cpp -g -O0 -pthread

install: all
	mkdir -p $(PREFIX)/include/chan
	cp -f $(SRC)/chan.h $(PREFIX)/include/chan/chan.h

uninstall:
	rm -rf $(PREFIX)/include/chan/chan.h

.PHONY: build check test clean install uninstall
