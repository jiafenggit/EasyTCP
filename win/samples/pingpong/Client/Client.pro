#-------------------------------------------------
#
# Project created by QtCreator 2017-06-03T16:26:12
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Client
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        widget.cpp \
    ../../../src/EasyTcpAcceptor.cpp \
    ../../../src/EasyTcpAutoBuffer.cpp \
    ../../../src/EasyTcpClient.cpp \
    ../../../src/EasyTcpConnection.cpp \
    ../../../src/EasyTcpContext.cpp \
    ../../../src/EasyTcpServer.cpp \
    ../../../src/EasyTcpWorker.cpp \
    ../../../src/EasyTcpWorkers.cpp

HEADERS  += widget.h \
    ../../../src/EasyTcpAcceptor.h \
    ../../../src/EasyTcpAutoBuffer.h \
    ../../../src/EasyTcpClient.h \
    ../../../src/EasyTcpConnection.h \
    ../../../src/EasyTcpContext.h \
    ../../../src/EasyTcpServer.h \
    ../../../src/EasyTcpWorker.h \
    ../../../src/EasyTcpWorkers.h \
    ../../test_config.h

INCLUDEPATH += ../../../src

LIBS += -lWs2_32

FORMS    += widget.ui
