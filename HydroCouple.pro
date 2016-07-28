#Author Caleb Amoa Buahin
#Email caleb.buahin@gmail.com
#Date 2016
#License GNU General Public License (see <http://www.gnu.org/licenses/> for details).


TEMPLATE = lib
VERSION = 1.0.0
TARGET = HydroCouple

QT += core

INCLUDEPATH += ./include

HEADERS += ./include/hydrocouple.h \
           ./include/hydrocoupletemporal.h \
           ./include/hydrocouplespatial.h \
           ./include/hydrocouplespatiotemporal.h

PRECOMPILED_HEADER += ./include/stdafx.h

SOURCES += ./src/stdafx.cpp


CONFIG(debug, debug|release) {
   DESTDIR = ./build/debug
   OBJECTS_DIR = $$DESTDIR/.obj
   MOC_DIR = $$DESTDIR/.moc
   RCC_DIR = $$DESTDIR/.qrc
   UI_DIR = $$DESTDIR/.ui
}

CONFIG(release, debug|release) {
    DESTDIR = lib
    RELEASE_EXTRAS = ./build/release 
    OBJECTS_DIR = $$RELEASE_EXTRAS/.obj
    MOC_DIR = $$RELEASE_EXTRAS/.moc
    RCC_DIR = $$RELEASE_EXTRAS/.qrc
    UI_DIR = $$RELEASE_EXTRAS/.ui
}        
