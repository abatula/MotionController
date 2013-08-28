/* main.cpp
 * Test the motion controller object
 */

#include <stdio.h>
#include <string>
#include <cstring>

#include "motionController.h"


int main(void)
{  
    MotionController *controller = new MotionController();
    bool initialized = false;
    bool motionRunning = false;
    initialized = controller->initMotionManager();
    usleep(1000000); // 1 second = 1,000,000 us
    
    if(initialized)
    {        
        printf("initialized\n");
        controller->initActionEditor();
        controller->executePage(15);
        motionRunning = controller->actionRunning();     
        while(motionRunning)
        {
            usleep(1000000);
            motionRunning = controller->actionRunning();
        }
        
        controller->executePage(1);
        motionRunning = controller->actionRunning();     
        while(motionRunning)
        {
            usleep(1000000);
            motionRunning = controller->actionRunning();
        }
        
        controller->executePage(15);
        motionRunning = controller->actionRunning();     
        while(motionRunning)
        {
            usleep(1000000);
            motionRunning = controller->actionRunning();
        }
        
        controller->initWalking();
        usleep(1000000);
        controller->walkForward();
        usleep(2000000);
        controller->stopWalking();
        usleep(2000000);
        
        controller->initActionEditor();
        controller->executePage(15);
        motionRunning = controller->actionRunning();     
        while(motionRunning)
        {
            usleep(1000000);
            motionRunning = controller->actionRunning();
        }
//             
        
    }
    else
        printf("\n Initialization failed\n");
}
