# DarkPlate - C++ demo of a license plate reader.
# Copyright 2021 Stephane Charette <stephanecharette@gmail.com>
# MIT license applies.  See "license.txt" for details.

ADD_EXECUTABLE (DarkPlate main.cpp)
TARGET_LINK_LIBRARIES (DarkPlate PRIVATE Threads::Threads ${DARKHELP} ${DARKNET} ${OpenCV_LIBS})

