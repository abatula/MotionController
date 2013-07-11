###############################################################
#
# Purpose: Makefile for "MotionController"
# Author.: robotis (modified by Alyssa Batula)
# Version: 0.1
# License: GPL
#
###############################################################

TARGET = MotionController

INCLUDE_DIRS = -I../../include -I../../../Framework/include

CXX = g++
CXXFLAGS += -O2 -g -DLINUX -Wall $(INCLUDE_DIRS)
#CXXFLAGS += -O2 -DLINUX -DDEBUG -Wall $(INCLUDE_DIRS)
LFLAGS += -lpthread -ljpeg -lrt

OBJECTS =   main.o motionController.o

all: $(TARGET)

clean:
	rm -f *.a *.o $(TARGET) core *~ *.so *.lo

darwin.a:
	make -C ../../build

$(TARGET): darwin.a $(OBJECTS)
	$(CXX) $(CFLAGS) $(LFLAGS) $(OBJECTS) ../../lib/darwin.a -o $(TARGET)
	chmod 755 $(TARGET)

# useful to make a backup "make tgz"
tgz: clean
	mkdir -p backups
	tar czvf ./backups/camera_`date +"%Y_%m_%d_%H.%M.%S"`.tgz --exclude backups *
