from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup
import pybind11
import sys

# macOS requires this linker flag for Python extensions
extra_link_args = ["-undefined", "dynamic_lookup"] if sys.platform == "darwin" else []

ext_modules = [
    Pybind11Extension(
        "my_service",
        ["bindings.cpp"],
        include_dirs=[pybind11.get_include()],
        cxx_std=17,
        extra_link_args=extra_link_args,  # ✅ critical for macOS
    ),
]

setup(
    name="my_service",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.8",
)
