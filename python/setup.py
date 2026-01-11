from setuptools import setup, Distribution

# A trivial class to tell setuptools that this is NOT a pure-python package
class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True

setup(
    name="pycube",
    version="0.1.0",
    packages=["pycube"],
    package_data={"pycube": ["*.so", "*.dylib", "*.dll"]},
    distclass=BinaryDistribution
)