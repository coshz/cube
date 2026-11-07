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
            ext.define_macros = list(ext.define_macros or []) + [
                ('CUBE_USE_SYMMETRY', '1'),
                ('CUBE_ENABLE_TRACE', '1'),
            ]
            if compiler_type == 'msvc': 
                extra = ['/std:c++17', '/O2', '/DNDEBUG']
            else:
                extra = ['-std=c++17', '-O3', '-DNDEBUG', '-fPIC']
            ext.extra_compile_args = list(ext.extra_compile_args or []) + extra
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