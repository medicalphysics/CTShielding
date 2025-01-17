cmake_minimum_required(VERSION 3.20)

project(CTShielding VERSION 0.0.1 DESCRIPTION "Carm scatter test" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Let's nicely support folders in IDE's
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

## Fetch packages
include(FetchContent)
## Adding DXMClib package
FetchContent_Declare(
    libdxmc
    GIT_REPOSITORY https://github.com/medicalphysics/DXMClib.git
    GIT_TAG develop
    )
FetchContent_MakeAvailable(libdxmc)

add_executable(ctshielding 
	src/main.cpp    
)

target_include_directories(ctshielding PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ctshielding PRIVATE	libdxmc)

#Adding data from dxmcCarm
dxmclib_add_physics_list(ctshielding)




