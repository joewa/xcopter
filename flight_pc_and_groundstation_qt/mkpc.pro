# #####################################################################
# Enumerator
# #####################################################################
PROJECT = mkpd
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += qextserialport \
    ../xc_shared/shared \
    ../xc_shared/sw_flight_pc
VPATH += ../xc_shared/shared \
    ../xc_shared/sw_flight_pc
QMAKE_LIBDIR += qextserialport/build
OBJECTS_DIR = .obj # avoid using obj here, on some systems 'make' will try to switch into obj from the current directory and you will get "Cannot find file" error
MOC_DIR = .moc
UI_DIR = .uic
CONFIG += qt \
    thread \
    warn_on \
    console
SOURCES += main.cpp \
    PortListener.cpp \
    serialthread.cpp \
    tinycomm.c \
    FlightHID.cpp \
    qmagellan.cpp \
    xdrvlib.c \
    difftool.c \
    xccontrol.cpp \
    Forms/dlg_flightinput.cpp
HEADERS += PortListener.h \
    serialthread.h \
    tinycomm.h \
    tinymsgdefs_fcxpc.h \
    FlightHID.h \
    qmagellan.h \
    xdrvlib.h \
    difftool.h \
    xccontrol.h \
    Forms/dlg_flightinput.h \
    fstream_tinymsgdefs_fcxpc.h \
    ../xc_shared/sw_flight_pc/xctypes.h
FORMS += Forms/dlg_flightinput.ui
CONFIG(debug, debug|release):LIBS += -lqextserialportd
else:LIBS += -lqextserialport
unix:DEFINES = _TTY_POSIX_
win32:DEFINES = _TTY_WIN_
