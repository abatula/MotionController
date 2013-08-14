/*
 * MotionController.cpp
 * Author:      Alyssa Batula
 * Created:     7/3/2013
 * Modified:    7/10/2013
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
 * Based off of the action_script demo written by robotis corp.
 * and the beat tracking code by Mark Koh.
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
#define MOTION_FILE_PATH    "../../../Data/motion_1024.bin"
#else
#define MOTION_FILE_PATH    "../../../Data/motion_4096.bin"
#endif

#define PORT 9930
#define BUFLEN 70
#define SAMPLE_RATE (44100)

MotionController::MotionController()
/* Constructor */
{
    managerInitialized = false; // Motion manager is initially not initialized
}

MotionController::~MotionController()
/* Destructor */
{
}

bool MotionController::initMotionManager()
/* 
 * Initialize the motion manager to control DARwIn-OP by selecting and running pages. 
 * Return True on success, otherwise return False
 */
{
    signal(SIGABRT, &MotionController::sighandler);
    signal(SIGTERM, &MotionController::sighandler);
    signal(SIGQUIT, &MotionController::sighandler);
    signal(SIGINT, &MotionController::sighandler);

    /* Declare server and socket variables */
//     struct sockaddr_in sad; //Structure to hold server IP Address // TODO: Do I need this?
//     struct sockaddr_in cad; //Structure to hold client IP Address // TODO: Do I need this?
//     int serverSocket; //Socket descriptor // TODO: Do I need this?
//     int slen=sizeof(cad); // TODO: Do I need this?
//     char buf[BUFLEN]; // TODO: Do I need this?

    // FIXME: I'm not sure this does anything
    MotionController::changeCurrentDir(); 

    Action::GetInstance()->LoadFile(MOTION_FILE_PATH);

    //////////////////// Framework Initialize ////////////////////////////
    LinuxCM730 linux_cm730("/dev/ttyUSB0");
    CM730 cm730(&linux_cm730);
    if(MotionManager::GetInstance()->Initialize(&cm730) == false)
    {
        return false; // Report failure to initialize (this isn't terribly uncommon, sometimes needs a retry)
    }

    MotionManager::GetInstance()->AddModule((MotionModule*)Action::GetInstance());
    LinuxMotionTimer *motion_timer = new LinuxMotionTimer(MotionManager::GetInstance());
    motion_timer->Start();
    /////////////////////////////////////////////////////////////////////
    
    MotionManager::GetInstance()->SetEnable(true); 
    
    managerInitialized = true; // Mark manager as initialized if successful
    
    return true;
}

void MotionController::executePage(int pageNum)
/* Execute the specified page number in the action editor */
{
    if(managerInitialized) // Don't run if the manager is not initialized
    {
        Action::GetInstance()->Start(pageNum);
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

// -------
void MotionController::changeCurrentDir()
{
    char exepath[1024] = {0};
    if(readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        chdir(dirname(exepath));
}

void MotionController::sighandler(int sig)
{
    struct termios term;
    Action::GetInstance()->Start(1);    /* Init(stand up) pose */
    tcgetattr( STDIN_FILENO, &term );
    term.c_lflag |= ICANON | ECHO;
    tcsetattr( STDIN_FILENO, TCSANOW, &term );

    exit(0);
}






