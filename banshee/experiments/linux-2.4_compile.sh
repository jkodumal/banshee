#!/bin/bash
export GCC_SUBST=gcc296
export CC="/moa/sc1/jkodumal/work/banshee/experiments/gcc_subst.py"
cd $1
yes '' | make config &> ../buildlogs/linux-2.4_$2.confout 
make dep &> ../buildlogs/linux-2.4_$2.depout 
make &> ../buildlogs/linux-2.4_$2.buildout
moke modules &> ../buildlogs/linux-2.4_$2.modout
make modules_install &> ../buildlogs/linux-2.4_$2.modinstout