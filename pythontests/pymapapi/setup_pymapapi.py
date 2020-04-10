from distutils.core import setup, Extension

module1 = Extension(
    "pymapapi",
    libraries=["qdmapacces", "converter"],
    library_dirs=["/usr/gisserver/"],
    sources=["pymapapi.c"],
)

setup(
    name="Python MapApi module",
    version="1.0",
    description="This is a demo package",
    ext_modules=[module1],
)
