#!/bin/bash

cd $1
./setup 
rm -rf tests
rm -rf examples
rm -rf experiments
make clean all CC="gcc -P -save-temps"
