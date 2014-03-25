from distutils.core import setup, Extension
from Cython.Build import cythonize
from Cython.Distutils import build_ext

setup(name='MotionController',
      version='0.0.1',
      description='DARwIn-OP MotionController class',
      author='Alyssa Batula',
      author_email='batulaa@drexel.edu',
      url='https://github.com/abatula/MotionController',
      ext_modules=[
            Extension('MotionController', ['MotionController.pyx', 'motionController.cpp'],
                language='c++',
                extra_objects=['/darwin/Linux/lib/darwin.a'], # Pass the Robotis darwin.a library to the linker (not supposed to add an extension, unsure if I need it or why)
                libraries=['jpeg', 'rt'], # Link with the jpeg and rt libraries
                include_dirs=['/darwin/Framework/include', '/darwin/Linux/include',] # Include DARwIn-OP files from Robotis
                )
            ],
      cmdclass={'build_ext':build_ext}
      )
      
