#!/bin/sh
setenv GCC_SUBST gcc296
cd $1
chmod -R +w *
yes '' | make config CC="/moa/sc1/jkodumal/work/banshee/experiments/gcc_subst.py -P -save-temps" &> ../buildlogs/linux-2.4_$2.confout  
make dep CC="/moa/sc1/jkodumal/work/banshee/experiments/gcc_subst.py -P -save-temps"
 &> ../buildlogs/linux-2.4_$2.depout 
make CC="/moa/sc1/jkodumal/work/banshee/experiments/gcc_subst.py -P -save-temps"
 &> ../buildlogs/linux-2.4_$2.buildout
moke modules CC="/moa/sc1/jkodumal/work/banshee/experiments/gcc_subst.py -P -save-temps"
 &> ../buildlogs/linux-2.4_$2.modout
make modules_install CC="/moa/sc1/jkodumal/work/banshee/experiments/gcc_subst.py -P -save-temps"
  &> ../buildlogs/linux-2.4_$2.modinstout