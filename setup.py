import shutil
import sys
from glob import glob

import setuptools
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

__version__ = '0.0.1'


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


extra_flags = []
if sys.platform == 'darwin':
    extra_flags = ['-stdlib=libc++', '-mmacosx-version-min=10.14']


# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            compiler.compile([f.name], extra_postargs=[flagname, *extra_flags])
        except setuptools.distutils.errors.CompileError:
            return False
    return True


def cpp_flag(compiler):
    """Return the -std=c++[11/14/17] compiler flag.
    The newer version is prefered over c++11 (when it is available).
    """
    flags = ['-std=c++17', '-std=c++14', '-std=c++11']

    for flag in flags:
        if has_flag(compiler, flag):
            return flag

    raise RuntimeError('Unsupported compiler -- at least C++11 support '
                       'is needed!')


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': extra_flags,
    }
    l_opts = {
        'msvc': ['-ltesseract'],
        'unix': ['-ltesseract'],
    }

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        opts.append(f'-DVERSION_INFO="{self.distribution.get_version()}"')
        link_opts = self.l_opts.get(ct, [])
        print(opts, link_opts)
        if ct == 'unix':
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, '-fvisibility=hidden'):
                opts.append('-fvisibility=hidden')
        for ext in self.extensions:
            ext.extra_compile_args = opts
            ext.extra_link_args = link_opts
        build_ext.build_extensions(self)


if __name__ == "__main__":
    ext = Extension(
        'pysseract',
        glob("src/*.cpp"),
        include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True),
            "src/"
        ],
        language='c++'
    )

    setup(
        name='Pysseract',
        version=__version__,
        author='Hongze Xia',
        author_email='hongzex@gmail.com',
        url='https://github.com/xiahongze/pysseract',
        description='Python binding to Tesseract API',
        long_description='',
        ext_modules=[ext],
        install_requires=['pybind11>=2.2'],
        setup_requires=['pybind11>=2.2'],
        cmdclass={'build_ext': BuildExt},
        zip_safe=False,
    )

    shutil.rmtree('var')  # cleanup
