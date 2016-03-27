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
