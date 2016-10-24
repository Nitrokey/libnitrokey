CC  = $(PREFIX)-gcc
#CXX = $(PREFIX)-g++
CXX = clang++-3.8
LD = $(CXX)

INCLUDE = -I../include -ICatch/single_include/
LIB = -L../build
LDLIBS = -lnitrokey
BUILD = build

CXXFLAGS = -std=c++14 -fPIC -Wno-gnu-variable-sized-type-not-at-end

CXXSOURCES = $(wildcard *.cc)
TARGETS = $(CXXSOURCES:%.cc=$(BUILD)/%)
DEPENDS = $(CXXSOURCES:%.cc=$(BUILD)/%.d)

$(BUILD)/%.d: %.cc
	$(CXX) -M $< -o $@ $(INCLUDE) $(CXXFLAGS)

$(BUILD)/%: %.cc $(DEPENDS)
	$(CXX) $< -o $@ $(INCLUDE) $(LIB) $(CXXFLAGS) $(LDLIBS)

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

mrproper: clean
	rm -f $(BUILD)/*.d

.PHONY: all clean mrproper

include $(wildcard build/*.d)
