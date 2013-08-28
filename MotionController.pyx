# distutils: language = c++
# distutils: sources = motionController.cpp

from libcpp cimport bool

cdef extern from "motionController.h":
    cdef cppclass MotionController:
        MotionController() except +
        bool initMotionManager()
        void initActionEditor()
        void initWalking()
        void executePage(int)
        bool actionRunning()
        void walkForward()
        void stopWalking()
        
cdef class PyMotionController:
    cdef MotionController *thisptr
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
    def walkForward(self):
        self.thisptr.walkForward()
    def stopWalking(self):
        self.thisptr.stopWalking()