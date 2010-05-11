NAME		:= cio-httpd
SOURCES		:= $(wildcard httpd/*.cpp)
CPPFLAGS	:= -I.
LIBS		:= -L$(O)/lib -lcio
DEPENDS		:= $(O)/lib/libcio.so

include build/binary.mk
