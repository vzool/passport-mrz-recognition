cmake_minimum_required (VERSION 2.6)
project (mrz)
MESSAGE( STATUS "PROJECT_NAME: " ${PROJECT_NAME} )

option(ENABLE_OPENCV "Build with OpenCV" OFF)
MESSAGE(STATUS "Build with OpenCV: ${ENABLE_OPENCV}")

# Check platforms
if (CMAKE_HOST_WIN32)
    set(WINDOWS 1)
elseif(CMAKE_HOST_UNIX)
    set(LINUX 1)
endif()

# Add search path for include and lib files
MESSAGE( STATUS "CPU architecture ${CMAKE_SYSTEM_PROCESSOR}" )
if(WINDOWS)
    link_directories("${PROJECT_SOURCE_DIR}/platform/windows/lib/") 
elseif(LINUX)
    link_directories("${PROJECT_SOURCE_DIR}/platform/linux/")
endif()
include_directories("${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/include/")

# Add the executable
if (ENABLE_OPENCV)
    find_package(OpenCV REQUIRED)
    add_executable(${PROJECT_NAME} mrzcv.cpp)
    if(WINDOWS)
        target_link_libraries (${PROJECT_NAME} "DynamsoftLabelRecognizerx64" ${OpenCV_LIBS})
    else()
        target_link_libraries (${PROJECT_NAME} "DynamsoftLabelRecognizer" ${OpenCV_LIBS})
    endif()
else()
    add_executable(${PROJECT_NAME} mrz.cpp)
    if(WINDOWS)
        target_link_libraries (${PROJECT_NAME} "DynamsoftLabelRecognizerx64")
    else()
        target_link_libraries (${PROJECT_NAME} "DynamsoftLabelRecognizer")
    endif()
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

