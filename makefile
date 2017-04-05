OBJECTS = $(patsubst %.cc,build/%.o,$(wildcard *.cc))
HEADERS = $(wildcard include/*.h)
OPTS = -g -Wall -Wextra -O3 -fPIC

ifeq ($(shell uname), Linux)
LIBS = -lhidapi-hidraw
else
LIBS = -lhidapi
endif

all: build/libnitrokey.so build/libnitrokey.a
.PHONY: all

build/libnitrokey.so: $(OBJECTS)
	$(CXX) -shared $(LIBS) $(OPTS) -o $@ $(OBJECTS)

build/libnitrokey.a: $(OBJECTS)
	ar rcs $@ $(OBJECTS)

build/%.o: %.cc $(HEADERS)
	$(CXX) -std=c++14 -I include $(OPTS) -c $< -o $@

clean:
	rm -f build/*.o build/*.a build/*.so
