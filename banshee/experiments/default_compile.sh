#!/bin/bash

cd $1
./setup && ./configure CC="/Users/jkodumal/work/oink_all/banshee/experiments/gcc_subst.py"
rm -rf tests
rm -rf examples
rm -rf experiments
make clean all CC="/Users/jkodumal/work/oink_all/banshee/experiments/gcc_subst.py -P -save-temps"
