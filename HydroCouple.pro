#Author Caleb Amoa Buahin
#Date 2016
#License


TEMPLATE = lib
VERSION = 1.0.0
TARGET = HydroCouple

QT += core

HEADERS += ./include/hydrocouple.h \
           ./include/hydrocoupletemporal.h \
           ./include/hydrocouplespatial.h

PRECOMPILED_HEADER += ./include/stdafx.h

SOURCES += ./src/stdafx.cpp
