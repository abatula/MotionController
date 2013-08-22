from distutils.core import setup, Extension
from Cython.Build import cythonize
from Cython.Distutils import build_ext

setup(name='MotionController', 
      ext_modules=[Extension('MotionController', ['MotionController.pyx', 'motionController.cpp'], 
                             language='c++',
                             extra_objects=['/darwin/Linux/lib/darwin.a'],
                             library_dirs=['/darwin/Linux/lib'],
                             libraries=['jpeg', 'rt'],
                             include_dirs=['/darwin/Framework/include',
                                            '/darwin/Linux/include',
                                            '/darwin/Linux',
                                            '/darwin/Framework/src',
                                            '/darwin/Framework/src/motion',
                                            '/darwin/Framework/src/motion/modules',
                                            '/darwin/Framework/src/vision',
                                            '/darwin/Framework/src/minIni',
                                            '/darwin/Framework/src/math',
                                            '/darwin/Linux/build',
                                            '/darwin/Linux/lib'
                                           ]
                             )],
      cmdclass={'build_ext':build_ext}
      )
      