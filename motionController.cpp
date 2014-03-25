/*
 * MotionController.cpp
 * Author:      Alyssa Batula
 *
 * Description: 
 * This class allows high level access and control of DARwIn-OP's movements 
 * through a motion controller object. This code supports using either the 
 * motion manager for pre-programmed motions or the walking manager.
 * 
 * Examples:
 * bool initMotionManager();
 * void initActionEditor();
 * void executePage(int pageNumber);
 * bool actionRunning();
 * 
 * 
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

// Do I need these 2?
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

#define INI_FILE_PATH       "/darwin/Data/config.ini"

#define PORT 9930
#define BUFLEN 70
#define SAMPLE_RATE (44100)

MotionController::MotionController()
/* Constructor */
{
  managerInitialized = false;         // Motion manager is initially not initialized
  actionEditorInitialized = false;    // Action editor is initially not initialized
  walkingInitialized = false;         // Walking is initially not initialized
  headInitialized = false;	      // Head is initially not initialized
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
 * Return True on success, otherwise return False. This must be run before 
 * running either initActionEditor(), initHead(), or initWalking(). It will attempt to 
 * initialize multiple times before giving up.
 */
{
  int MAX_ATTEMPTS = 5;
  int currentAttempt = 0;
  // FIXME: I'm not sure this does anything
  MotionController::changeCurrentDir(); 

  if(Action::GetInstance()->LoadFile(MOTION_FILE_PATH) == false)
    {
      printf("Failed to load motion file %s", MOTION_FILE_PATH);
      return false;
    }

  // Try to initialize multiple times
  while((currentAttempt < MAX_ATTEMPTS) && (managerInitialized == false))
    {
      if(MotionManager::GetInstance()->Initialize(static_cast<CM730*> (cm730)) == true)
        {
	  managerInitialized = true; // Mark manager as initialized if successful
        }
      else
        {
	  currentAttempt++;
	  printf("Initialization failed, attempt %i", currentAttempt);
	  usleep(1000000); // Wait before trying to initialize again
        }
        
    }
    
  // Only start the timer if successfully initialized
  if (managerInitialized)
    {
      LinuxMotionTimer *motion_timer = new LinuxMotionTimer(MotionManager::GetInstance());
      motion_timer->Start(); 
    }
    
  // Initialize to the start position
  minIni* ini = new minIni(INI_FILE_PATH);

  MotionManager::GetInstance()->LoadINISettings(ini);
  Action::GetInstance()->m_Joint.SetEnableBody(true, true);
  MotionManager::GetInstance()->SetEnable(true);

  Action::GetInstance()->Start(15) // Go to the sitting position
    while(Action::GetInstance()->IsRunning()) usleep(8*1000);
    
  return managerInitialized;
}

void MotionController::initActionEditor()
{
  /*
   * Set up the action editor module to control DARwIn-OP by running pages. 
   * Make sure that the motion manager has already been initialized first
   */
  if(managerInitialized)
    {
      MotionManager::GetInstance()->AddModule((MotionModule*)Action::GetInstance());
      MotionManager::GetInstance()->SetEnable(true);
      // Set action editor and walking flags
      actionEditorInitialized = true;
      walkingInitialized = false;
    }
  else
    {
      printf("Motion manager is not initialized. Run initMotionManager() first");
    }
}

void MotionController::initWalking()
{
  /*
   * Get DARwIn ready for walking. Make sure motion manager has been 
   * initialized first.
   */
    
  if(managerInitialized)
    {
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
      // Walking::GetInstance()->m_Joint.SetEnableBody(true);
      // MotionManager::GetInstance()->SetEnable(true);
      // TODO: Do I need the previous two lines?
        
      Walking::GetInstance()->Initialize();
        
      // Set action editor and walking flags
      actionEditorInitialized = false;
      walkingInitialized = true;
    }
    
  else
    {
      printf("Motion manager is not initialized. Run initMotionManager() first");
    }
}

void MotionController::initHead()
{
  /*
   * Get DARwIn ready for head movement. Make sure motion manager has been 
   * initialized first.
   */
    
  if(managerInitialized)
    {
      MotionManager::GetInstance()->AddModule((MotionModule*)Head::GetInstance());
      MotionManager::GetInstance()->SetEnable(true);
      headInitialized = true;
    }
    
  else
    {
      printf("Motion manager is not initialized. Run initMotionManager() first");
    }
}

void MotionController::executePage(int pageNum)
/* Execute the specified page number in the action editor */
{
    
  // start of original
  if(actionEditorInitialized) // Don't run if the manager is not initialized
    {
      Action::GetInstance()->Start(pageNum);
    }
  else
    {
      printf("Action Editor is not initialized, run initActionEditor()");
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
  if(walkingInitialized)
    {
      Walking::GetInstance()->A_MOVE_AMPLITUDE = direction;
      Walking::GetInstance()->Start();
        
      if(duration >= 0.0)
        {
	  usleep(duration*1000000);
	  Walking::GetInstance()->Stop();
        }
    }
  else
    {
      printf("Walking is not initialized, run initWalking()");
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
  if(walkingInitialized)
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
  else
    {
      printf("Walking is not initialized, run initWalking()");
    }
        
}

void MotionController::stopWalking()
/*
 * Stop walking
 */
{
  if(walkingInitialized)
    {
      Walking::GetInstance()->Stop();
    }
  else
    {
      printf("Walking is not initialized, run initWalking()");
    }
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
  /* 
   * Signal handling 
   *   
   * Sit down if the process is ended
   */
  struct termios term;
  Action::GetInstance()->Start(15);    // Sit Down  
  while(Action::GetInstance()->IsRunning()) usleep(8*1000) // Wait until the motion has finished
					      tcgetattr( STDIN_FILENO, &term );
  term.c_lflag |= ICANON | ECHO;
  tcsetattr( STDIN_FILENO, TCSANOW, &term );

  exit(0);
}






