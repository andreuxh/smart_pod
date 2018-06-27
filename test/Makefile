CXX = g++
CXXFLAGS += -std=c++11 -O2 -Wall
CPPFLAGS += -I../include

all: test
	./test

test: test.cc ../include/smart_pod.hh
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $<
