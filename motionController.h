#ifndef MOTIONCONTROLLER__
#define MOTIONCONTROLLER_
class MotionController {
    bool managerInitialized;
    LinuxCM730 *linux_cm730;
    CM730 *cm730; 
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
