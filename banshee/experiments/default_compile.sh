#!/bin/bash

cd $1
./setup && ./configure CC="/moa/sc1/jkodumal/work/banshee/experiments/gcc_subst.py"
rm -rf tests
rm -rf examples
rm -rf experiments
make clean all CC="/moa/sc1/jkodumal/work/banshee/experiments/gcc_subst.py -P -save-temps"
