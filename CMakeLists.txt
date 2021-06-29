cmake_minimum_required (VERSION 2.6)
project (mrz)
MESSAGE( STATUS "PROJECT_NAME: " ${PROJECT_NAME} )

# Check platforms
if (CMAKE_HOST_WIN32)
    set(WINDOWS 1)
elseif(CMAKE_HOST_UNIX)
    set(LINUX 1)
endif()

# Set RPATH
# if(WINDOWS)
# else()
#     SET(CMAKE_CXX_FLAGS "-std=c++11 -O3")
#     set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#     MESSAGE( STATUS "CMAKE_INSTALL_RPATH: " "${CMAKE_INSTALL_PREFIX}/lib" )
#     set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# endif()

# Add search path for include and lib files
MESSAGE( STATUS "CPU architecture ${CMAKE_SYSTEM_PROCESSOR}" )
if(WINDOWS)
    link_directories("${PROJECT_SOURCE_DIR}/platform/windows/lib/") 
elseif(LINUX)
    link_directories("${PROJECT_SOURCE_DIR}/platform/linux/")
endif()
include_directories("${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/include/")

# Add the executable
add_executable(${PROJECT_NAME} mrz.cpp)
if(WINDOWS)
    target_link_libraries (${PROJECT_NAME} "DynamsoftLabelRecognitionx64")
else()
    target_link_libraries (${PROJECT_NAME} "DynamsoftLabelRecognition")
endif()

# Copy DLLs
if(WINDOWS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/platform/windows/bin/"      
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

# Copy template
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/template/"      
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# Copy model files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/CharacterModel"      
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/CharacterModel)

