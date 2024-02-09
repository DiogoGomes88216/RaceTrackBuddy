QT += core
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        cfilesaver.cpp \
        cgnss.cpp \
        cimu.cpp \
        dpvaestimate.cpp \
        main.cpp \
        shared_memory.cpp

HEADERS += \
    cfilesaver.h \
    cgnss.h \
    cimu.h \
    dpvaestimate.h \
    shared_memory.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /etc/RaceTrackBuddy/Daemons
!isEmpty(target.path): INSTALLS += target

