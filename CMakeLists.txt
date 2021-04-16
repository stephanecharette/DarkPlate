# DarkPlate - C++ demo of a license plate reader.
# Copyright 2021 Stephane Charette <stephanecharette@gmail.com>
# MIT license applies.  See "license.txt" for details.


CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

PROJECT (DarkPlate C CXX)

IF (NOT DEFINED CMAKE_BUILD_TYPE)
	SET (CMAKE_BUILD_TYPE Release)
ENDIF ()

SET (CMAKE_CXX_STANDARD				17	)
SET (CMAKE_CXX_STANDARD_REQUIRED	ON	)
SET (CMAKE_CXX_EXTENSIONS			OFF	)

ADD_DEFINITIONS ("-Wall -Wextra -Werror -Wno-unused-parameter -Wshadow")

FIND_PACKAGE (Threads	REQUIRED)
FIND_PACKAGE (OpenCV	REQUIRED)
FIND_LIBRARY (DARKHELP	darkhelp)
FIND_LIBRARY (DARKNET	darknet)

INCLUDE_DIRECTORIES (${OpenCV_INCLUDE_DIRS})

ADD_SUBDIRECTORY (src)

