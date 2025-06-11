cmake_minimum_required(VERSION 3.10)
project(SPECT2D_Simulation)

# Require C++11 or newer
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Geant4
find_package(Geant4 REQUIRED ui_all vis_all)

# Optionally find ROOT if you use it for output
find_package(ROOT QUIET)

# Include paths
include(${Geant4_USE_FILE})

# Define sources
set(SOURCES
    main.cc
    DetectorConstruction.cc
    PrimaryGeneratorAction.cc
    PhysicsList.cc
    EventAction.cc
    RunAction.cc
    SteppingAction.cc
    SensitiveDetector.cc
    SimMessenger.cc
)

# Define executable
add_executable(SPECT2D ${SOURCES})

# Link with Geant4 libraries
target_link_libraries(SPECT2D ${Geant4_LIBRARIES})

# If using ROOT, link it as well
if(ROOT_FOUND)
    include_directories(${ROOT_INCLUDE_DIRS})
    target_link_libraries(SPECT2D ${ROOT_LIBRARIES})
endif()

# Installation target
install(TARGETS SPECT2D DESTINATION bin)

# Optionally create a build directory for output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

