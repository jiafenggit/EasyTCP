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
    ../../../src/EasyTcpEventPoll.cpp \
    ../../../src/EasyTcpEventPolls.cpp \
    ../../../src/EasyTcpContext_linux.cpp \
    ../../../src/EasyTcpConnection_linux.cpp \
    ../../../src/EasyTcpClient_linux.cpp \
    ../../../src/EasyTcpServer_linux.cpp \
    ../../../src/EasyTcpWorker.cpp \
    ../../../src/EasyTcpContext_win.cpp \
    ../../../src/EasyTcpConnection_win.cpp \
    ../../../src/EasyTcpClient_win.cpp \
    ../../../src/EasyTcpIOCP.cpp \
    ../../../src/EasyTcpIOCPs.cpp \
    ../../../src/EasyTcpServer_win.cpp

HEADERS  += widget.h \
    ../../../src/EasyTcpAcceptor.h \
    ../../../src/EasyTcpAutoBuffer.h \
    ../../test_config.h \
    ../../../src/EasyTcpEventPoll.h \
    ../../../src/EasyTcpEventPolls.h \
    ../../../src/EasyTcpDef.h \
    ../../../src/EasyTcpContext_linux.h \
    ../../../src/EasyTcpConnection_linux.h \
    ../../../src/EasyTcpClient_linux.h \
    ../../../src/EasyTcpServer_linux.h \
    ../../../src/EasyTcpWorker.h \
    ../../../src/EasyTcpIServer.h \
    ../../../src/EasyTcpIConnection.h \
    ../../../src/EasyTcpIClient.h \
    ../../../src/EasyTcp.h \
    ../../../src/EasyTcpContext_win.h \
    ../../../src/EasyTcpConnection_win.h \
    ../../../src/EasyTcpClient_win.h \
    ../../../src/EasyTcpIOCPs.h \
    ../../../src/EasyTcpIOCP.h \
    ../../../src/EasyTcpServer_win.h


INCLUDEPATH += ../../../src

FORMS    += widget.ui

windows:{
    LIBS += -lWs2_32
}
