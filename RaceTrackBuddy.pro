QT += quick virtualkeyboard

CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++0x
LIBS += -DWITH_FFMPEG=ON
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Backend/ccam.cpp \
        Backend/claptimer.cpp \
        Backend/cmainprocess.cpp \
        Backend/csvFile.cpp \
        Backend/shared_memory.cpp \
        main.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /etc/$${TARGET}
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Backend/ccam.h \
    Backend/claptimer.h \
    Backend/cmainprocess.h \
    Backend/csvFile.h \
    Backend/shared_memory.h

CONFIG += link_pkgconfig
PKGCONFIG += opencv
DISTFILES +=
