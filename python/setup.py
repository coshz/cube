import os 
from setuptools import setup, Distribution
from setuptools.command.build_py import build_py


# A trivial class to tell setuptools that this is NOT a pure-python package
class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True


class CustomBuildPy(build_py):
    def run(self):
        super().run()
        self.stage_native_libraries()
        
    def stage_native_libraries(self): 
        sys_lib_dir = os.environ.get("PYCUBE_LIB_DIR")
        libs = ['libcube.so', 'libcube.dylib', 'cube.dll']
        
        for lib in libs: 
            spath = os.path.join(sys_lib_dir,lib)
            if os.path.exists(spath):
                target_dir = os.path.join(self.build_lib, 'pycube')
                os.makedirs(target_dir, exist_ok=True)
                self.copy_file(spath,os.path.join(target_dir,lib)) 
                break
        else:
            print("Warning: `PYCUBE_LIB_DIR` not set or empty. Wheel might miss native libs!")


setup(
    name="pycube",
    version="0.1.1",
    packages=["pycube"],
    cmdclass={'build_py': CustomBuildPy},
    distclass=BinaryDistribution,
    zip_safe=False
)