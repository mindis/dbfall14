#!/bin/bash
#unzip $1.zip
#cd $1
cd codebase
cd rbf
make clean
make
./rbftest
./rbftest11a
./rbftest11b
./rbftest12
./rbftest13
./rbftest14
./rbftest15
./rbftest16
