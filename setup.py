from setuptools import setup, Extension

mods = [Extension('ctok', sources = ['ctok.c'])]

setup(
    name='ctok',
    version='0.0',
    description="Expose CPython's tokenizer as a Python class",
    ext_modules=mods,
    setup_requires=["pytest-runner"],
    tests_require=["pytest"],
)
