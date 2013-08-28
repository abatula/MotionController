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
        
    controller.initWalking()
    time.sleep(1)
    controller.walk(5, 0, 0)
    time.sleep(1)
    controller.walk(5, 10)
    time.sleep(1)
    controller.walk(5, -10)
    time.sleep(1)
    controller.walk(-1, 0, 2)
    
    time.sleep(5)
    controller.stopWalking()
    time.sleep(2)
    
    controller.initActionEditor()
    controller.executePage(15)
    
    while controller.actionRunning():
        time.sleep(0.5)
else:
    print('Initialization Failed')

    
   