cmake_minimum_required(VERSION 3.25)
project(sijson LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

find_package(sireflect CONFIG REQUIRED)

add_library(sijson
    src/sijson_error.c
    src/sijson_parser.c
    src/sijson_reflect.c
    src/sijson_value.c
    src/sijson_writer.c)
add_library(sijson::sijson ALIAS sijson)
target_compile_features(sijson PUBLIC c_std_17)
set_target_properties(sijson PROPERTIES C_STANDARD 17 C_STANDARD_REQUIRED YES)
target_include_directories(sijson PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(sijson PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_definitions(sijson PRIVATE sijson_EXPORTS)
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(sijson PUBLIC sijson_STATIC)
endif()
target_link_libraries(sijson PUBLIC sireflect::sireflect)
if(UNIX)
    target_link_libraries(sijson PRIVATE m)
endif()

install(TARGETS sijson EXPORT sijsonTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES include/sijson.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES include/sijson/export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sijson)
install(EXPORT sijsonTargets NAMESPACE sijson:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sijson)
configure_package_config_file(cmake/sijsonConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sijsonConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sijson)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sijsonConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sijson)

