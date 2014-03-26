#-------------------------------------------------
#
# Project created by QtCreator 2014-03-06T15:03:27
#
#-------------------------------------------------

QT       += core gui network sql

QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IcebreakerViewer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    SettingDialog.cpp

HEADERS  += mainwindow.h \
    SettingDialog.h

FORMS    += mainwindow.ui \
    SettingDialog.ui
