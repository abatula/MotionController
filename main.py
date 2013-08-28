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
    controller.initActionEditor()
    controller.executePage(15)

    while controller.actionRunning():
        time.sleep(0.5)
        
    controller.executePage(1)

    while controller.actionRunning():
        time.sleep(0.5)
        
    controller.executePage(15)

    while controller.actionRunning():
        time.sleep(0.5)
        
    controller.initWalking()
    time.sleep(1)
    controller.walkForward()
    time.sleep(2)
    controller.stopWalking()
    time.sleep(2)
    
    controller.initActionEditor()
    controller.executePage(15)
    
    while controller.actionRunning():
        time.sleep(0.5)
else:
    print('Initialization Failed')

    
   