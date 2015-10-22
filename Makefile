CC  = $(PREFIX)-gcc
CXX = $(PREFIX)-g++
LD = $(CXX)

INCLUDE = -Iinclude/
LIB = -lhidapi-libusb
BUILD = build

CXXFLAGS = -std=c++14 -fPIC
SOFLAGS = -shared

CXXSOURCES = $(wildcard *.cc)
OBJ = $(CXXSOURCES:%.cc=$(BUILD)/%.o)

$(BUILD)/libnitrokey.so: $(OBJ)
	$(CXX) $(SOFLAGS) $(OBJ) -o $@

$(BUILD)/%.o: %.cc
	$(CXX) -c $< -o $@ $(INCLUDE) $(CXXFLAGS)

all: $(OBJ) $(BUILD)/libnitrokey.so

clean:
	rm -f $(OBJ)
	rm -f $(BUILD)/libnitrokey.so
	make -C unittest clean

mrproper: clean
	rm -f $(BUILD)/*.d

unittest: all
	make -C unittest
	cd unittest/build && ln -fs ../../build/libnitrokey.so .

.PHONY: all clean mrproper unittest

include $(wildcard build/*.d)
