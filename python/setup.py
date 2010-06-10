import distutils.core

ext = distutils.core.Extension(
	"_cio",
	sources = ["_cio.c"],
	include_dirs = [".."],
	library_dirs = ["../debug/lib"],
	libraries = ["cio"],
)

distutils.core.setup(
	name = "cio",
	py_modules = ["cio"],
	ext_modules = [ext],
)
