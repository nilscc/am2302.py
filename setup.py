import setuptools
import Cython.Build

class BinaryDistribution(setuptools.Distribution):
    def is_pure(self):
        return False

setuptools.setup(
    name='am2302',
    version='0.1.0',
    distclass=BinaryDistribution,

    ext_modules=Cython.Build.cythonize([
        setuptools.Extension(
            name='am2302',
            language='c++',
            sources=['am2302.pyx'],
            include_dirs=['inc'],
            #library_dirs=['lib'],
            libraries=['wiringPi'],
            #define_macros=[],
            #extra_compile_args=[],
        ),
    ], language_level=3),
)
