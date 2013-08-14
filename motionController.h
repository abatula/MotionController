#ifndef MOTIONCONTROLLER__
#define MOTIONCONTROLLER_
class MotionController {
    public:
        MotionController();
        ~MotionController();
        bool initMotionManager();
        void executePage(int);
        bool actionRunning();
    protected:
        bool managerInitialized;
        void changeCurrentDir();
        static void sighandler(int);
    
};
#endif
