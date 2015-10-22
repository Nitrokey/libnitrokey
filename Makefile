CC  = $(PREFIX)-gcc
CXX = $(PREFIX)-g++
LD = $(CXX)

INCLUDE = -Iinclude/
LIB = -lhidapi-libusb
BUILD = build

CXXFLAGS = -std=c++14

CXXSOURCES = $(wildcard *.cc)
OBJ = $(CXXSOURCES:%.cc:$(BUILD)/%.o)

$(BUILD)/libnitrokey.so: $(OBJ)
	$(CXX) -shared $(OBJ) -o $@

$(BUILD)/%.o: %.cc
	$(CXX) -c $< -o $@ $(INCLUDE) $(CXXFLAGS)

all: $(OBJ) $(BUILD)/libnitrokey.so

clean:
	rm $(OBJ)
	rm $(BUILD)/libnitrokey.so

mrproper: clean
	rm $(BUILD)/*.d

.PHONY: all clean mrproper

include $(wildcard build/*.d)
