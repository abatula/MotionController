from distutils.core import setup, Extension
from Cython.Build import cythonize
from Cython.Distutils import build_ext

setup(name='MotionController', 
      ext_modules=[Extension('MotionController', ['MotionController.pyx', 'motionController.cpp'], 
                             language='c++',
                             extra_objects=['/darwin/Linux/lib/darwin.a'],
                             libraries=['jpeg', 'rt'],
                             include_dirs=['/darwin/Framework/include',
                                            '/darwin/Linux/include',
                                           ]
                             )],
      cmdclass={'build_ext':build_ext}
      )
      