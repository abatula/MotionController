#ifndef MOTIONCONTROLLER__
#define MOTIONCONTROLLER_
class MotionController {
    bool managerInitialized;
    public:
        MotionController();
        ~MotionController();
        bool initMotionManager();
        void executePage(int);
        bool actionRunning();
    protected:
        static void changeCurrentDir();
        static void sighandler(int);
    
};
#endif
