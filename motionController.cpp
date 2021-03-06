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
  ini = new minIni(INI_FILE_PATH);

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
  MotionManager::GetInstance()->LoadINISettings(static_cast<minIni*> (ini));
  Action::GetInstance()->m_Joint.SetEnableBody(true, true);
  MotionManager::GetInstance()->SetEnable(true);

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
        if(actionEditorInitialized == false)
        {
            MotionManager::GetInstance()->AddModule((MotionModule*)Action::GetInstance());
            MotionManager::GetInstance()->SetEnable(true);
            // Set action editor flag
            actionEditorInitialized = true;
        }

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
      Walking::GetInstance()->LoadINISettings(static_cast<minIni*> (ini));

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
      // TODO: Do I need the previous two lines? Walking did not enable all the needed joints when they were both commented, but I'm unsure if/how much they interfere with action editor or head movements

      Walking::GetInstance()->Initialize();

      // Set walking flag
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
      Head::GetInstance()->LoadINISettings(static_cast<minIni*> (ini));
      Head::GetInstance()->Initialize();
      
      MotionManager::GetInstance()->SetEnable(true);
      headInitialized = true;
    }
    
  else
    {
      printf("Motion manager is not initialized. Run initMotionManager() first");
    }
}

void MotionController::moveHeadToHome()
/* Move head to home position via MoveToHome() function in Head class */
{
  if(headInitialized)		// Don't run if head is not initialized
    {
      MotionStatus::m_CurrentJoints.SetEnableBody(false);
      MotionStatus::m_CurrentJoints.SetEnableHeadOnly(true);
      Head::GetInstance()->MoveToHome();
    }
  else
    {
      printf("Head is not initialized, run initHead()");
    }
}

void MotionController::moveHeadByAngle(double pan, double tilt)
/* 
 * Move the head to the specified pan and tilt angles 
 *
 * Call the MoveByAngle function in the Head class
 *
 * INPUT:
 *    pan  - Pan angle
 *    tilt - tilt angle
 */
{
  if(headInitialized)		// Don't run if head is not initialized
    {
        MotionStatus::m_CurrentJoints.SetEnableBody(false);
        MotionStatus::m_CurrentJoints.SetEnableHeadOnly(true);
        Head::GetInstance()->MoveByAngle(pan, tilt);
    }
  else
    {
      printf("Head is not initialized, run initHead()");
    }
}

void MotionController::moveHeadByOffset(double pan, double tilt)
/* 
 * Move the head by the specified pan and tilt angle offsets from the current position
 *
 * Call the MoveByOffset function in the Head class
 *
 * INPUT:
 *    pan  - Pan angle offset
 *    tilt - tilt angle offset
 */
{
  if(headInitialized)		// Don't run if head is not initialized
    {
        MotionStatus::m_CurrentJoints.SetEnableBody(false);
        MotionStatus::m_CurrentJoints.SetEnableHeadOnly(true);
        Head::GetInstance()->MoveByAngleOffset(pan, tilt);
    }
  else
    {
      printf("Head is not initialized, run initHead()");
    }
}

double MotionController::getHeadTopLimitAngle()
/* Get the top angle limit for the head */
{
  if(headInitialized)		// Don't run if head is not initialized
    {
      return Head::GetInstance()->GetTopLimitAngle();
    }
  else
    {
      printf("Head is not initialized, run initHead()");
      return 0.0;
    }
}

double MotionController::getHeadBottomLimitAngle()
/* Get the bottom angle limit for the head */
{
  if(headInitialized)		// Don't run if head is not initialized
    {
      return Head::GetInstance()->GetBottomLimitAngle();
    }
  else
    {
      printf("Head is not initialized, run initHead()");
      return 0.0;
    }
}

double MotionController::getHeadLeftLimitAngle()
/* Get the left angle limit for the head */
{
  if(headInitialized)		// Don't run if head is not initialized
    {
      return Head::GetInstance()->GetLeftLimitAngle();
    }
  else
    {
      printf("Head is not initialized, run initHead()");
      return 0.0;
    }
}

double MotionController::getHeadRightLimitAngle()
/* Get the top angle limit for the head */
{
  if(headInitialized)		// Don't run if head is not initialized
    {
      return Head::GetInstance()->GetRightLimitAngle();
    }
  else
    {
      printf("Head is not initialized, run initHead()");
      return 0.0;
    }
}

double MotionController::getHeadPanAngle()
/* Get the current pan angle for the head */
{
  if(headInitialized)		// Don't run if head is not initialized
    {
      return Head::GetInstance()->GetPanAngle();
    }
  else
    {
      printf("Head is not initialized, run initHead()");
      return 0.0;
    }
}

double MotionController::getHeadTiltAngle()
/* Get the current tilt  angle for the head */
{
  if(headInitialized)		// Don't run if head is not initialized
    {
      return Head::GetInstance()->GetTiltAngle();
    }
  else
    {
      printf("Head is not initialized, run initHead()");
      return 0.0;
    }
}

void MotionController::executePage(int pageNum)
/* Execute the specified page number in the action editor */
{
    
  // start of original
  if(actionEditorInitialized) // Don't run if the manager is not initialized
    {
      MotionStatus::m_CurrentJoints.SetEnableBody(true);
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
      // Make sure lower body is enabled, but don't move the head if the head control is initialized
      MotionStatus::m_CurrentJoints.SetEnableBody(true);
      if(headInitialized)
      {
          Walking::GetInstance()->m_Joint.SetEnableHeadOnly(false);
      }
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
      // Make sure lower body is enabled, but don't move the head if the head control is initialized
      MotionStatus::m_CurrentJoints.SetEnableBody(true);
      if(headInitialized)
      {
          Walking::GetInstance()->m_Joint.SetEnableHeadOnly(false);
      }
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
      // Make sure lower body is enabled, but don't move the head if the head control is initialized
      MotionStatus::m_CurrentJoints.SetEnableBody(true);
      if(headInitialized)
      {
          Walking::GetInstance()->m_Joint.SetEnableHeadOnly(false);
      }
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
    */
    struct termios term;
    tcgetattr( STDIN_FILENO, &term );
    term.c_lflag |= ICANON | ECHO;
    tcsetattr( STDIN_FILENO, TCSANOW, &term );

    exit(0);
}






