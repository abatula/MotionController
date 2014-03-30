# distutils: language = c++
# distutils: sources = motionController.cpp

from libcpp cimport bool

cdef extern from "motionController.h": # Make class definition available
    cdef cppclass MotionController:    # Add the C++ class
        MotionController() except +    # Allow Cython to raise Python error if one occurs
        bool initMotionManager()
        void initActionEditor()
        void initWalking()
        void executePage(int)
        bool actionRunning()
        void walk(double,double)
        void walk(double,double,double)
        void stopWalking()

# Create the Python wrapper class
cdef class PyMotionController:
    cdef MotionController *thisptr # Create a C++ pointer to the MotionController object
    def __cinit__(self):
        self.thisptr = new MotionController()
    def __dealloc__(self):
        del self.thisptr
    def initMotionManager(self):
        return self.thisptr.initMotionManager()
    def initActionEditor(self):
        self.thisptr.initActionEditor()
    def initWalking(self):
        self.thisptr.initWalking()
    def executePage(self, pageNum):
        self.thisptr.executePage(pageNum)
    def actionRunning(self):
        return self.thisptr.actionRunning()
    def walk(self, duration=-1.0, direction=0.0, stepSize=-1):
        if stepSize == -1:
            self.thisptr.walk(duration, direction)
        else:
            self.thisptr.walk(duration, direction, stepSize)
    def stopWalking(self):
        self.thisptr.stopWalking()