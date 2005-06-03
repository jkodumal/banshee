OS:=$(shell uname)

ifeq ($(OS), Darwin)
	jni_dir = /System/Library/Frameworks/JavaVM.framework/Headers 
else 
	jni_dir = /usr/local/java/include/
endif

ifndef RLIB
region_dir = ../libcompat
else
region_dir = ../rlib
endif

engine_dir = ../engine

dyckcfl_dir = ../dyckcfl

all: java jdyckcfl

.PHONY: java jdyckcfl

java:
	javac `find . -name \*.java`

jdyckcfl: java 
	javah -jni banshee.dyckcfl.UnsafeDyckCFL
	$(CC) -c -I. -I$(dyckcfl_dir) -I$(engine_dir) -I$(region_dir) -I$(jni_dir) -Ddeletes= \
	-Dtraditional= -Dsameregion= -Dparentptr= -DNONSPEC -Wall \
	banshee/dyckcfl/UnsafeDyckCFLImp.c
	$(CC) -dynamiclib -o libjdyckcfl.jnilib $(dyckcfl_dir)/dyckcfl.o UnsafeDyckCFLImp.o $(engine_dir)/libnsengine.a $(region_dir)/libregions.a -framework JavaVM

clean:
	rm -f *.o libjdyckcfl.jnilib
