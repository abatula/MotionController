MotionController
======

Class for high-level control of the DARwIn-OP using the action editor/motion manager.

This code belongs in a subfolder of /darwin/Linux/project/MotionController on the DARwIn-OP. 

If you have permissions errors (either running in C++ or Python) you may need to run it with sudo for admin privileges. 

System Requirements
================
C++ Only
* None

Python wrapper
* python-dev
* Cython

Note that at this time, DARwIn comes with Ubuntu and uses Python2 by default. So all dependencies refer to Python2-compatible versions where applicable. 

Installation
================
C++
------
To compile, run 'make' in the MotionController folder. 

To test the installation, first kill the demo script that runs at startup:

    $ pkill demo

then run the example code with:

    $ ./MotionController
    
or, if you need admin privelidges:

    $ sudo ./MotionController

Modify main.cpp to run your own program to control DARwIn.

Python
------
To compile/install locally, run:

    $ python setup.py build_ext --inplace
    
or to install system-wide:

    $ python setup.py build_ext
    
To test the installation, first kill the demo script that runs at startup:

    $ pkill demo

Then run the main.py script:

    $ python main.py
    
or

    $ sudo python main.py

Modify the main.py file, or write your own to control DARwIn from Python. Just import MotionController and initialize a controller:

    import MotionController
    controller = MotionController.PyMotionController()
    
Then use the methods in the controller object to control the robot.

It should be relatively easy to update this code for Python3 if you choose to install it on your robot, or if future versions come with Python3 out-of-the-box.

Notes
================
If you do not kill the demo program using before running this script, it may cause segmentation faults. DARwIn's MotionManager (which is used by the MotionController class) is a [singleton](http://en.wikipedia.org/wiki/Singleton_pattern) and does not deal well with two programs trying to use it at once.