/*
 * MotionController.cpp
 * Author:      Alyssa Batula
 *
 * Description: 
 * This class allows high level access and control of DARwIn-OP's movements 
 * through a motion controller object. Initialize the motion manager, execute 
 * a specified page, and check if a motion is finished executing.
 * 
 * Examples:
 * bool initMotionManager();
 * void executePage(int pageNumber);
 * bool actionRunning();
 *
 */

#include <unistd.h>
#include <string.h>
#include <string>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

// Do I need these 3?
#include "Point.h"
#include "mjpg_streamer.h"
#include "minIni.h"

#include "Action.h"
#include "Head.h"
#include "Walking.h"
#include "MX28.h"
#include "MotionManager.h"
#include "LinuxMotionTimer.h"
#include "LinuxCM730.h"
#include "LinuxActionScript.h"

#include "motionController.h"

#ifdef MX28_1024
#define MOTION_FILE_PATH    "/darwin/Data/motion_1024.bin"
#else
#define MOTION_FILE_PATH    "/darwin/Data/motion_4096.bin"
#endif

#define INI_FILE_PATH       "/darwin/Data/config.ini" // FIXME: Do i need this?

#define PORT 9930
#define BUFLEN 70
#define SAMPLE_RATE (44100)

MotionController::MotionController()
/* Constructor */
{
    managerInitialized = false; // Motion manager is initially not initialized
    linux_cm730 = new LinuxCM730("/dev/ttyUSB0"); // Create objects for linux CM730 controller
    cm730 = new CM730(static_cast<LinuxCM730*> (linux_cm730));     // Create new object for CM730 controller
    
    // Set up signal handlers
    signal(SIGABRT, &MotionController::sighandler);
    signal(SIGTERM, &MotionController::sighandler);
    signal(SIGQUIT, &MotionController::sighandler);
    signal(SIGINT, &MotionController::sighandler);
}

MotionController::~MotionController()
/* Destructor */
{
}

bool MotionController::initMotionManager()
/* 
 * Initialize the motion manager to control DARwIn-OP. 
 * Return True on success, otherwise return False
 */
{

    // FIXME: I'm not sure this does anything
    MotionController::changeCurrentDir(); 

    if(Action::GetInstance()->LoadFile(MOTION_FILE_PATH) == false)
    {
        printf("Failed to load motion file %s", MOTION_FILE_PATH);
        return false;
    }

    if(MotionManager::GetInstance()->Initialize(static_cast<CM730*> (cm730)) == false)
    {
        return false; // Report failure to initialize (this isn't terribly uncommon, sometimes needs a retry)
    }
    
    LinuxMotionTimer *motion_timer = new LinuxMotionTimer(MotionManager::GetInstance());
    motion_timer->Start(); 
    
    managerInitialized = true; // Mark manager as initialized if successful
    return true;
}

void MotionController::initActionEditor()
{
    /*
     * Set up the action editor module to control DARwIn-OP by running pages
     */
    
    MotionManager::GetInstance()->AddModule((MotionModule*)Action::GetInstance());
    MotionManager::GetInstance()->SetEnable(true);
}

void MotionController::initWalking()
{
    /*
     * Get DARwIn ready for walking
     */
    
    minIni* ini = new minIni(INI_FILE_PATH);
    Walking::GetInstance()->LoadINISettings(ini);
    
    MotionManager::GetInstance()->AddModule((MotionModule*)Walking::GetInstance());
    
    // Have DARwIn get to walk ready position SLOWLY
    int n = 0;
    int param[JointData::NUMBER_OF_JOINTS * 5];
    int wGoalPosition, wStartPosition, wDistance;

    for(int id=JointData::ID_R_SHOULDER_PITCH; id<JointData::NUMBER_OF_JOINTS; id++)
    {
        wStartPosition = MotionStatus::m_CurrentJoints.GetValue(id);
        wGoalPosition = Walking::GetInstance()->m_Joint.GetValue(id);
        if( wStartPosition > wGoalPosition )
            wDistance = wStartPosition - wGoalPosition;
        else
            wDistance = wGoalPosition - wStartPosition;

        wDistance >>= 2;
        if( wDistance < 8 )
            wDistance = 8;

        param[n++] = id;
        param[n++] = CM730::GetLowByte(wGoalPosition);
        param[n++] = CM730::GetHighByte(wGoalPosition);
        param[n++] = CM730::GetLowByte(wDistance);
        param[n++] = CM730::GetHighByte(wDistance);
    }
    (static_cast<CM730*>(cm730))->SyncWrite(MX28::P_GOAL_POSITION_L, 5, JointData::NUMBER_OF_JOINTS - 1, param);
    
    usleep(1000000); // Give DARwIn time to get to the position
    
    // Enable walking and the motion manager
    Walking::GetInstance()->m_Joint.SetEnableBody(true);
    MotionManager::GetInstance()->SetEnable(true);
    
    Walking::GetInstance()->Initialize();
}

void MotionController::executePage(int pageNum)
/* Execute the specified page number in the action editor */
{
    
    // start of original
    if(managerInitialized) // Don't run if the manager is not initialized
    {
        Action::GetInstance()->Start(pageNum);
    }
    else
    {
        printf("Motion manager must be initialized!");
    }
}

bool MotionController::actionRunning()
/* 
 * Check whether the action is still running.
 * Return True if the robot is still moving, otherwise return False.
 */
{
    return Action::GetInstance()->IsRunning();
}

void MotionController::walk(double duration, double direction)
/*
 * Walk in the specified direction for the given amount of time
 * 
 * duration - walk for the specified number of seconds (-1 for continuous walking)
 * direction - Walk forward (0), turn left (positive) or turn right (negative)
 */
{
    Walking::GetInstance()->A_MOVE_AMPLITUDE = direction;
    Walking::GetInstance()->Start();
    
    if(duration >= 0.0)
    {
        usleep(duration*1000000);
        Walking::GetInstance()->Stop();
    }
        
}

void MotionController::walk(double duration, double direction, double stepSize)
/*
 * Walk in the specified direction for the given amount of time with the specified stepSize
 * 
 * duration - walk for the specified number of seconds (-1 for continuous walking)
 * direction - Walk forward (0), turn left (positive) or turn right (negative)
 * stepSize - Size of step to take
 */
{
    Walking::GetInstance()->A_MOVE_AMPLITUDE = direction;
    Walking::GetInstance()->X_MOVE_AMPLITUDE = stepSize;
    Walking::GetInstance()->Start();
    
    if(duration >= 0.0)
    {
        usleep(duration*1000000);
        Walking::GetInstance()->Stop();
    }
        
}

void MotionController::stopWalking()
/*
 * Stop walking
 */
{
    Walking::GetInstance()->Stop();
}

// -------
void MotionController::changeCurrentDir()
{
    /* Still not sure what this does, but it's in the Robotis code */
    char exepath[1024] = {0};
    if(readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        chdir(dirname(exepath));
}

void MotionController::sighandler(int sig)
{
    /* Signal handling */
    struct termios term;
    Action::GetInstance()->Start(1);    /* Init(stand up) pose */
    tcgetattr( STDIN_FILENO, &term );
    term.c_lflag |= ICANON | ECHO;
    tcsetattr( STDIN_FILENO, TCSANOW, &term );

    exit(0);
}






