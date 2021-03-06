# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (Tutorials)

find_package(OpenGL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
    message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
    message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()



# Compile external dependencies
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
    add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)



include_directories(
        external/AntTweakBar-1.16/include/
        external/glfw-3.1.2/include/
        external/glm-0.9.7.1/
        external/glew-1.13.0/include/
        external/assimp-3.0.1270/include/
        external/bullet-2.81-rev2613/src/
        .
)

set(ALL_LIBS
        ${OPENGL_LIBRARY}
        glfw
        GLEW_1130
        )

add_definitions(
        -DTW_STATIC
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D
        -DGLEW_STATIC
        -D_CRT_SECURE_NO_WARNINGS
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

# Tutorial 2
add_executable(HW1
        HW1/main.cpp
        common/shader.cpp
        common/shader.hpp

        HW1/FirstTriangle.shader
        HW1/SecondTriangle.shader
        HW1/SimpleVertexShader.vertexshader
        )
target_link_libraries(HW1
        ${ALL_LIBS}
        )
# Xcode and Visual working directories
set_target_properties(HW1 PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/HW1/")
create_target_launcher(HW1 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/HW1/")
create_default_target_launcher(HW1 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/HW1/") # tut 1 is not the default or people would complain that tut 2 doesn't work


# Tutorial 3
add_executable(HW2
        HW2/main.cpp
        common/shader.cpp
        common/shader.hpp

        HW2/FirstTriangle.shader
        HW2/SecondTriangle.shader
        HW2/SimpleTransform.vertexshader
        )
target_link_libraries(HW2
        ${ALL_LIBS}
        )
# Xcode and Visual working directories
set_target_properties(HW2 PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/HW2/")
create_target_launcher(HW2 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/HW2/") # Visual


# Tutorial 4
add_executable(HW3
        HW3/main.cpp
        common/shader.cpp
        common/shader.hpp

        HW3/TransformVertexShader.vertexshader
        HW3/ColorFragmentShader.fragmentshader
        )
target_link_libraries(HW3
        ${ALL_LIBS}
        )
# Xcode and Visual working directories
set_target_properties(HW3 PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/HW3/")
create_target_launcher(HW3 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/HW3/")




SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )


if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
    add_custom_command(
            TARGET HW1 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/HW1${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/HW1/"
    )
    add_custom_command(
            TARGET HW2 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/HW2${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/HW2/"
    )
    add_custom_command(
            TARGET HW3 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/HW3${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/HW3/"
    )

elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

