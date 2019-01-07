#!/bin/bash
gpg2 --decrypt Makefile.gpg | wc -lc
sleep 1
./test_137 -s
sleep 1
gpg2 --decrypt Makefile.gpg | wc -lc
sleep 1
./test_137 -s
gpg2 --version
git describe --long
