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
    def walk(self, duration=-1.0, direction=0.0, stepSize=-1):
        if stepSize == -1:
            self.thisptr.walk(duration, direction)
        else:
            self.thisptr.walk(duration, direction, stepSize)
    def stopWalking(self):
        self.thisptr.stopWalking()
    def getHeadAngleLimits():
        top = self.thisptr.getHeadTopLimitAngle()
	bottom = self.thisptr.getHeadBottomLimitAngle()
	left = self.thisptr.getHeadLeftLimitAngle()
	right = self.thisptr.getHeadRightLimitAngle()
	return(top,bottom,left,right)
    def getHeadPanTiltAngles():
        pan = self.thisptr.getHeadPanAngle()
	tilt = self.thisptr.getHeadTiltAngle()
	return (pan,tilt)