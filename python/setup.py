import distutils.core
import glob
import os

libdir = os.path.join(os.getenv("O", "../debug"), "lib")

ext = distutils.core.Extension(
	"_cio",
	sources = glob.glob("_cio/*.c"),
	include_dirs = [".."],
	library_dirs = [libdir],
	libraries = ["cio"],
)

distutils.core.setup(
	name = "cio",
	py_modules = ["cio"],
	ext_modules = [ext],
)
