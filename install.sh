#!/bin/bash

# Author: Caleb Buahin
# Purpose: The purpose of this script is to install all of HydroCouple and its dependencies on Linux (Debian), Mac
# Copyright: Caleb Buahin 2014-2018

#Before installing
#*******************************************************************************
# Make sure this file has execute privileges.
# $ chmod u+x install.sh
#
# Compilers for C/C++ are needed in order to install the libraries
# used by HydroCouple. Here we use the GNU Compiler Collection (GCC). One can make sure
# that all necessary compilers are installed by executing:
# $ which gcc
# $ which g++
# $ which gfortran
#
# If one or more of the compilers is missing, execute:
# $ yum install gcc g++ gfortran (Red Hat)
# $ apt-get install gcc g++ gfortran (Debian)
# $ brew install gcc --enable-cxx (Mac)


#Default arguments
#*******************************************************************************
FORCE_INSTALL_QT = false


#Install Qt 5.7.0 and dependencies





