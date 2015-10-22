CC  = $(PREFIX)-gcc
CXX = $(PREFIX)-g++
LD = $(CXX)

INCLUDE = -I../include
LIB = -L../build
LDLIBS = -lhidapi-libusb -lnitrokey
BUILD = build

CXXFLAGS = -std=c++14 -fPIC

CXXSOURCES = $(wildcard *.cc)
TARGETS = $(CXXSOURCES:%.cc=$(BUILD)/%)

$(BUILD)/%: %.cc
	$(CXX) $< -o $@ $(INCLUDE) $(LIB) $(CXXFLAGS) $(LDLIBS)

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

mrproper: clean
	rm -f $(BUILD)/*.d

.PHONY: all clean mrproper

include $(wildcard build/*.d)
