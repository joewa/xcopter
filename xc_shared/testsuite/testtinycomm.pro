# #####################################################################
# Enumerator
# #####################################################################
PROJECT = testtinycomm
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += ../shared \
    ../sw_flight_pc
VPATH += ../xc_shared/shared \
    ../xc_shared/sw_flight_pc
OBJECTS_DIR = .obj # avoid using obj here, on some systems 'make' will try to switch into obj from the current directory and you will get "Cannot find file" error
CONFIG += qtcore \
    thread \
    warn_on \
    console
SOURCES += ../shared/tinycomm.c
HEADERS += ../shared/tinycomm.h \
    ../shared/tinymsgdefs_fcxpc.h \
    ../sw_flight_pc/xctypes.h
CONFIG(debug, debug|release)
unix:DEFINES = _TTY_POSIX_
win32:DEFINES = _TTY_WIN_

DEFINES += COMMDEBUG
