from distutils.core import setup
from Cython.Build import cythonize

setup(name='MotionController', ext_modules=cythonize('*.pyx'), include_dirs=['/darwin/Framework/include','/darwin/Linux/include']
      )