#-------------------------------------------------
#
# Project created by QtCreator 2016-07-08T15:35:52
#
#-------------------------------------------------

#-------------------------------------------------
#
# 广东海洋大学
#
# 三体队四轴飞行器地面站 Qt 程序源码
#
# 作者：陈旭然
#
# 队员：李铭士、陈旭然、姚友业
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Fly
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    spcomm.cpp \
    imgthread.cpp \
    flycontrol.cpp \
    flyparams.cpp

HEADERS  += mainwindow.h \
    spcomm.h \
    imgthread.h \
    flycontrol.h \
    flyparams.h

FORMS    += mainwindow.ui

RC_FILE += icon.rc

include($(QT_DIR)/config/x8.config)
