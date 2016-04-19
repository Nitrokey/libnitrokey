# Build

To build the lib on ubuntu 14.04LTS you need to

1. Install

  * catch

2. Run

   mkdir build
   make CXXFLAGS="-std=c++14 -fPIC -stdlib=libc++" all 

# Test

To run the tests, you need the nitrokey hardware with the default
administrator password 12345678.

To run the tests, change to the directory unittest/build and run

   LD_LIBRARY_PATH=. ./test_HOTP  
