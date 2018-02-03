#-------------------------------------------------
#
# Project created by QtCreator 2016-06-15T14:28:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = poisson
TEMPLATE = app


SOURCES += main.cpp\
        poisson.cpp \
    poisson_run.cpp \
    poisson_type.cpp \
    poisson_dst.cpp

HEADERS  += poisson.h

FORMS    += poisson.ui

INCLUDEPATH+=F:\Work\CMAKE\QT\opencv\include\opencv\
           F:\Work\CMAKE\QT\opencv\include\opencv2\
           F:\Work\CMAKE\QT\opencv\include
LIBS+=F:\Work\CMAKE\QT\opencv\lib\libopencv_*.a

RESOURCES += \
    pix.qrc
