/* main.cpp
 * Test the motion controller object
 */

#include <stdio.h>
#include <string>
#include <cstring>

#include "motionController.h"


int main(void)
{  
    MotionController controller;
    bool initialized = false;
    bool motionRunning = false;
    initialized = controller.initMotionManager();
    usleep(1000000); // 1 second = 1,000,000 us
    
    if(initialized)
    {        
        controller.executePage(60);
        motionRunning = controller.actionRunning();     
        while(motionRunning)
        {
            usleep(1000000);
            motionRunning = controller.actionRunning();
        }
        
        controller.executePage(61);
        motionRunning = controller.actionRunning();     
        while(motionRunning)
        {
            usleep(1000000);
            motionRunning = controller.actionRunning();
        }
        
        controller.executePage(60);
        motionRunning = controller.actionRunning();     
        while(motionRunning)
        {
            usleep(1000000);
            motionRunning = controller.actionRunning();
        }
            
//         printf("\n before sleep\n");
//         usleep(5*1000000); // 1 second = 1,000,000 us
//         printf("\n after sleep\n");
//         motionRunning = controller->actionRunning();
//         if(motionRunning)
//             printf("\n Motion is running");
//         else
//             printf("\n Motion ended");
        
    }
    else
        printf("\n Initialization failed\n");
}
