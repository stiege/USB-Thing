# Set minimum CMake version
cmake_minimum_required(VERSION 2.8.4)

# Configure project and languages
project(libusbthing C CXX)

set(CMAKE_C_FLAGS "-std=gnu11 -g")
set(CMAKE_CXX_FLAGS "-std=gnu++11 -g")

# Add project headers
# Add project headers
include_directories(${PROJECT_SOURCE_DIR}/../common/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

find_package(PkgConfig)
pkg_check_modules(LIBUSB libusb-1.0)

link_directories(${LIBUSB_LIBRARY_DIRS})
include_directories(${LIBUSB_INCLUDE_DIRS}/..)

set(USBTHING_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/source/usbthing.c)

# Add libraries
add_library(usbthing SHARED ${USBTHING_SOURCES})
target_link_libraries(usbthing ${LIBUSB_LIBRARIES})

install(TARGETS usbthing LIBRARY DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/usbthing)

