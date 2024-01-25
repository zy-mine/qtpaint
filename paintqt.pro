QT       += core gui sql



greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    downloadthread.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    switchbtn.cpp

HEADERS += \
    downloadthread.h \
    mainwindow.h \
    qcustomplot.h \
    switchbtn.h

FORMS += \
    mainwindow.ui \
    switchbtn.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

contains(QT_ARCH,x86_64){
    message("64-bit")
}else{
    message("32-bit")
    include($$PWD\gsl\gsl.pri)
}

