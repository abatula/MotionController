#ifndef MOTIONCONTROLLER__
#define MOTIONCONTROLLER_
class MotionController {
    bool managerInitialized;
    bool actionEditorInitialized;
    bool walkingInitialized;
    void *linux_cm730;
    void *cm730; 
    public:
        MotionController();
        ~MotionController();
        bool initMotionManager();
        void initActionEditor();
        void initWalking();
        void executePage(int);
        bool actionRunning();
        void walk(double, double);
        void walk(double, double, double);
        void stopWalking();
    protected:
        static void changeCurrentDir();
        static void sighandler(int);
    
};
#endif
