#!/bin/bash

cd $1
aclocal
autoconf
automake
./configure CC="/home/eecs/jkodumal/work/pldi05_experiments/banshee/experiments/gcc_subst.py"
make clean all CC=CC="/home/eecs/jkodumal/work/pldi05_experiments/banshee/experiments/gcc_subst.py"