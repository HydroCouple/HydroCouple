#Author Caleb Amoa Buahin
#Email caleb.buahin@gmail.com
#Date 2014-2019
#License GNU General Public License (see <http://www.gnu.org/licenses/> for details).
#Copyright 2014-2020, Caleb Buahin, All rights reserved.

TEMPLATE = lib
VERSION = 1.0.2
TARGET = HydroCouple

CONFIG += c++11
CONFIG += debug_and_release

QT += core

INCLUDEPATH += ./include

HEADERS += ./include/hydrocouple.h \
           ./include/hydrocoupletemporal.h \
           ./include/hydrocouplespatial.h \
           ./include/hydrocouplespatiotemporal.h \
           ./include/hydrocouplespatialwkb.h

PRECOMPILED_HEADER += ./include/stdafx.h

SOURCES += ./src/stdafx.cpp

CONFIG(debug, debug|release){
   DESTDIR = ./build/debug
   OBJECTS_DIR = $$DESTDIR/.obj
   MOC_DIR = $$DESTDIR/.moc
   RCC_DIR = $$DESTDIR/.qrc
   UI_DIR = $$DESTDIR/.ui
}

CONFIG(release, debug|release){

    #MacOS
    macx{
     DESTDIR = lib/macx
    }

    #Linux
    linux{
     DESTDIR = lib/linux
    }

    #Windows
    win32{
     DESTDIR = lib/win32
    }

    RELEASE_EXTRAS = ./build/release 
    OBJECTS_DIR = $$RELEASE_EXTRAS/.obj
    MOC_DIR = $$RELEASE_EXTRAS/.moc
    RCC_DIR = $$RELEASE_EXTRAS/.qrc
    UI_DIR = $$RELEASE_EXTRAS/.ui
}        
