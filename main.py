from __future__ import print_function, division

import time

import MotionController

controller = MotionController.PyMotionController()

initialized = False

try:
    initialized = controller.initMotionManager()
except:
    print('\nException occured')

if initialized:
    print("Initialized")
    
    controller.executePage(60)

    motionRunning = controller.actionRunning()
    print(str(motionRunning))
    while motionRunning:
        time.sleep(0.5)
        motionRunning = controller.actionRunning()
        print(str(motionRunning))
    
    #controller->executePage(61);
    #motionRunning = controller->actionRunning();     
    #while(motionRunning)
    #{
        #usleep(1000000);
        #motionRunning = controller->actionRunning();
    #}
    
    #controller->executePage(60);
    #motionRunning = controller->actionRunning();     
    #while(motionRunning)
    #{
        #usleep(1000000);
        #motionRunning = controller->actionRunning();
    #}