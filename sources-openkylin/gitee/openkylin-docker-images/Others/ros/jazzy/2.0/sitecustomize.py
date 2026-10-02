"""Enable setuptools' Python 3.12 distutils compatibility shim."""

try:
    import _distutils_hack
except ImportError:
    pass
else:
    _distutils_hack.add_shim()
