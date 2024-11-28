#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
get_filename_component(ProjectId ${CMAKE_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

# additional compiler flags
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USER_FLAGS}" )
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USER_FLAGS}" )

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.c*)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h*)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(${PROJECT_NAME} main.cc ${sources} ${headers})
target_link_libraries(${PROJECT_NAME} ${Geant4_LIBRARIES})


#
# link copy over macros
set(RUNFILES
    run
    gdml
)

foreach(_file ${RUNFILES})
    if(NOT EXISTS ${PROJECT_BINARY_DIR}/${_file})
        add_custom_command(TARGET ${ProjectId} POST_BUILD
            COMMAND ln -sf ${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}
            COMMENT "Creating symbolic link to ${_file} in ${PROJECT_BINARY_DIR}"
        )
    endif()
endforeach()

#----------------------------------------------------------------------------
# For internal Geant4 use - but has no effect if you build this
# example standalone
#
#add_custom_target(G4ET DEPENDS G4ET)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
#install(TARGETS G4ET DESTINATION bin)

