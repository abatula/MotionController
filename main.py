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
    
    controller.executePage(15)

    while controller.actionRunning():
        time.sleep(0.5)
        
    controller.executePage(1)

    while controller.actionRunning():
        time.sleep(0.5)
        
    controller.executePage(15)

    while controller.actionRunning():
        time.sleep(0.5)

    
   