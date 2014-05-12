.PHONY: all clean
.DEFAULT: all

CXX=g++
CXXFLAGS=-std=c++0x -Wall -Werror -pedantic -fPIC
LDFLAGS=-shared -ldl

OBJS=nettracer.o
DEPS=$(OBJS:.o=.d)

TARGET=libnettracer.so

all: $(TARGET)

$(TARGET): $(OBJS) $(DEPS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

%.d: %.cpp
	$(CXX) -MM $< > $@
	$(CXX) -MM $< | sed s/\\.o/.d/ >> $@

clean:
	-rm $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
