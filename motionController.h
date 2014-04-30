#ifndef MOTIONCONTROLLER__
#define MOTIONCONTROLLER__
class MotionController {
    bool managerInitialized;
    bool actionEditorInitialized;
    bool walkingInitialized;
    bool headInitialized;
    void *linux_cm730;
    void *cm730; 
    void *ini;
    public:
        MotionController();
        ~MotionController();
        bool initMotionManager();
        void initActionEditor();
        void initWalking();
	void initHead();
        void executePage(int);
        bool actionRunning();
        void walk(double, double);
        void walk(double, double, double);
        void stopWalking();
	double getHeadTopLimitAngle();
	double getHeadBottomLimitAngle();
	double getHeadLeftLimitAngle();
	double getHeadRightLimitAngle();
	double getHeadPanAngle();
	double getHeadTiltAngle();
	void moveHeadToHome();
	void moveHeadByAngle(double, double);
	void moveHeadByOffset(double, double);
	
    protected:
        static void changeCurrentDir();
        static void sighandler(int);
    
};
#endif
