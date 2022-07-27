cmake_minimum_required(VERSION 3.21)
project(id_tech_1_doom C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_VERBOSE_MAKEFILE ON)

# ---- DEFINES ---- #

#target_compile_definitions(
#        id_tech_1_doom
#        PRIVATE
#        <DEFINES HERE>
#)

# ---- SOURCES ---- #

# Include source content
file(GLOB_RECURSE sourceFiles CONFIGURE_DEPENDS "src/*.c")
file(GLOB_RECURSE headerFiles CONFIGURE_DEPENDS "src/*.h")

set(includeDirs "")
foreach(_headerFile ${headerFiles})
    get_filename_component(_dir ${_headerFile} PATH)
    list(APPEND includeDirs ${_dir})
endforeach()
list(REMOVE_DUPLICATES includeDirs)

# Mark executable
add_executable(id_tech_1_doom ${sourceFiles})
target_include_directories(id_tech_1_doom PRIVATE ${includeDirs})

# ----- LIBS ---- #

# PkgConfig
find_package(PkgConfig REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)
target_include_directories(id_tech_1_doom PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(id_tech_1_doom PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(id_tech_1_doom PUBLIC ${OPENGL_LIBRARIES})

# GLFW 3
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(id_tech_1_doom PRIVATE glfw)

# GLAD
find_package(glad CONFIG REQUIRED)
target_link_libraries(id_tech_1_doom PRIVATE glad::glad)

# GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(id_tech_1_doom PRIVATE glm::glm)