cmake_minimum_required(VERSION 3.22)

set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_MAX_CONCURRENCY 12)

if (WIN32)
	set(VCPKG_TARGET_TRIPLET x64-windows)
	include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
elseif (UNIX)
	set(VCPKG_TARGET_TRIPLET x64-linux)
 	include("/home/$ENV{USER}/GameDev/Libraries/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

project(bitsery
        LANGUAGES CXX
        VERSION 5.2.4)

#======== build options ===================================
option(BITSERY_BUILD_EXAMPLES "Build examples" ON)
option(BITSERY_BUILD_TESTS "Build tests" ON)

#============= setup target ======================
add_library(bitsery INTERFACE)
# create alias, so that user could always write target_link_libraries(... Bitsery::bitsery)
# despite of bitsery target is imported or not
add_library(Bitsery::bitsery ALIAS bitsery)

find_package(EASTL CONFIG REQUIRED)
target_link_libraries(bitsery INTERFACE EASTL)

include(GNUInstallDirs)
target_include_directories(bitsery INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(bitsery INTERFACE
        cxx_auto_type
        cxx_constexpr
        cxx_lambdas
        cxx_nullptr
        cxx_variadic_templates)

#=============== setup installation =======================
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/BitseryConfigVersion.cmake
        COMPATIBILITY SameMajorVersion)
install(TARGETS bitsery
        EXPORT bitseryTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT bitseryTargets
        FILE "BitseryConfig.cmake"
        NAMESPACE Bitsery::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bitsery)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/BitseryConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bitsery)
install(DIRECTORY include/bitsery
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#================ handle sub-projects =====================

if (BITSERY_BUILD_EXAMPLES)
    message("build bitsery examples")
    add_subdirectory(examples)
else()
    message("skip bitsery examples")
endif()

if (BITSERY_BUILD_TESTS)
    message("build bitsery tests")
    enable_testing()
    add_subdirectory(tests)
else()
    message("skip bitsery tests")
endif()
