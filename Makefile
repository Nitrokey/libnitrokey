CC  = $(PREFIX)-gcc
#CXX = $(PREFIX)-g++
CXX = clang++-3.8
LD = $(CXX)

INCLUDE = -Iinclude/
LIB = -lhidapi-libusb
BUILD = build

CXXFLAGS = -std=c++14 -fPIC -Wno-gnu-variable-sized-type-not-at-end
SOFLAGS = -shared

CXXSOURCES = $(wildcard *.cc)
OBJ = $(CXXSOURCES:%.cc=$(BUILD)/%.o)
DEPENDS = $(CXXSOURCES:%.cc=$(BUILD)/%.d)

all: $(OBJ) $(BUILD)/libnitrokey.so unittest

lib: $(OBJ) $(BUILD)/libnitrokey.so

$(BUILD)/libnitrokey.so: $(OBJ) $(DEPENDS)
	$(CXX) $(SOFLAGS) $(OBJ) $(LIB) -o $@

$(BUILD)/%.d: %.cc
	$(CXX) -M $< -o $@ $(INCLUDE) $(CXXFLAGS)

$(BUILD)/%.o: %.cc $(DEPENDS)
	$(CXX) -c $< -o $@ $(INCLUDE) $(CXXFLAGS)

clean:
	rm -f $(OBJ)
	rm -f $(BUILD)/libnitrokey.so
	${MAKE} -C unittest clean

mrproper: clean
	rm -f $(BUILD)/*.d
	${MAKE} -C unittest mrproper

unittest: $(BUILD)/libnitrokey.so
	${MAKE} -C unittest
	cd unittest/build && ln -fs ../../build/libnitrokey.so .

.PHONY: all clean mrproper unittest

include $(wildcard build/*.d)
