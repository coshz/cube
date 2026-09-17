from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import os 
import platform 


if platform.system() == "Darwin":
    os.environ.setdefault("MACOSX_DEPLOYMENT_TARGET", "10.15")


class CustomBuildExt(build_ext):
    def build_extensions(self): 
        compiler_type = self.compiler.compiler_type 
        for ext in self.extensions: 
            if compiler_type == 'msvc': 
                ext.extra_compile_args = ['/std:c++17', '/O2']
            else:
                ext.extra_compile_args = ['-std=c++17', '-O2', '-fPIC']
        super().build_extensions()


native_moudle = Extension(
    name="pycube._native",
    sources=[ "cxx/cube_amalg.min.cpp"],
    include_dirs=["cxx"],
    language="c++",
)

setup(
    ext_modules=[native_moudle],
    cmdclass={"build_ext": CustomBuildExt},
)