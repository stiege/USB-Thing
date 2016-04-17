#Add common information
include(${CMAKE_CURRENT_LIST_DIR}/../common/common.cmake)

#Add usbthing driver sources
set(USBTHING_SOURCES 
	${CMAKE_CURRENT_LIST_DIR}/source/usbthing.c
	)

#Add required inclusions
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

find_package(PkgConfig)
pkg_check_modules(LIBUSB libusb-1.0)

link_directories(${LIBUSB_LIBRARY_DIRS})
include_directories(${LIBUSB_INCLUDE_DIRS}/..)

#Create usbthing driver library
add_library(usbthing-driver ${USBTHING_SOURCES})
target_link_libraries(usbthing-driver ${LIBUSB_LIBRARIES})

#Add to convenience variable
set(LIBS ${LIBS} usbthing-driver usb-1.0)