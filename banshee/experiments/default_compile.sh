#!/bin/bash

cd $1
./setup 
make clean all CC="gcc -P -save-temps"