class MotionController {
    public:
        bool managerInitialized;
        MotionController();
        ~MotionController();
        int initMotionManager();
        void executePage(int);
        bool actionRunning();
    protected:
        void changeCurrentDir();
        void sighandler(int);
    
};