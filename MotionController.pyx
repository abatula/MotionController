# distutils: language = c++
# distutils: sources = motionController.cpp

from libcpp cimport bool

cdef extern from "motionController.h": # Make class definition available
    cdef cppclass MotionController:    # Add the C++ class
        MotionController() except +    # Allow Cython to raise Python error if one occurs
        bool initMotionManager()
        void initActionEditor()
        void initWalking()
        void initHead()
        void executePage(int)
        bool actionRunning()
        void walk(double,double)
        void walk(double,double,double)
        void stopWalking()
        double getHeadTopLimitAngle()
        double getHeadBottomLimitAngle()
        double getHeadLeftLimitAngle()
        double getHeadRightLimitAngle()
        double getHeadPanAngle()
        double getHeadTiltAngle()
        void moveHeadToHome()
        void moveHeadByAngle(double, double)
        void moveHeadByOffset(double, double)

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
    def initHead(self):
        self.thisptr.initHead()
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
    def getHeadAngleLimits(self):
        top = self.thisptr.getHeadTopLimitAngle()
        bottom = self.thisptr.getHeadBottomLimitAngle()
        left = self.thisptr.getHeadLeftLimitAngle()
        right = self.thisptr.getHeadRightLimitAngle()
        return(top,bottom,left,right)
    def getHeadPanTiltAngles(self):
        pan = self.thisptr.getHeadPanAngle()
        tilt = self.thisptr.getHeadTiltAngle()
        return (pan,tilt)
    def moveHeadToHome(self):
        self.thisptr.moveHeadToHome()
    def moveHead(self, pan, tilt, mode='direct'):
        if mode == 'direct':
            self.thisptr.moveHeadByAngle(pan, tilt)
        elif mode == 'offset':
            self.thisptr.moveHeadByOffset(pan, tilt)
        else:
            print('Invalid option ' + mode + ' for moveHead')
