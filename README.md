# Passport MRZ Recognition
A C/C++ sample of Passport MRZ recognition implemented with [Dynamsoft OCR SDK](https://www.dynamsoft.com/label-recognition/overview).

## Requirements
- [Visual Studio](https://www.visualstudio.com/downloads/)
- [CMake](https://cmake.org/download/)
- [Dynamsoft Label Recognition SDK](https://www.dynamsoft.com/label-recognition/downloads)
- [OpenCV 4.5.0](https://opencv.org/releases/)

    Configuration for CMake:

    ```
    # Find OpenCV, you may need to set OpenCV_DIR variable
    # to the absolute path to the directory containing OpenCVConfig.cmake file
    # via the command line or GUI
    find_package(OpenCV REQUIRED)
    ```

## 30-day FREE Trial License
Get a [free trial license](https://www.dynamsoft.com/customer/license/trialLicense?product=dlr) and save it to `license.txt`.

## How to Build and Run

![passport mrz recognition](https://www.dynamsoft.com/blog/wp-content/uploads/2021/06/passport-mrz-recognition.png)

### Windows
1. Copy `*.lib` files to `platform/windows/lib` folder and copy `*.dll` files to `platform/windows/bin` folder.

2. Create a build folder:

    ```
    mkdir build
    cd build
    ```

3. Configure the project.

    - Command-line app

        ```
        cmake -DCMAKE_GENERATOR_PLATFORM=x64 ..
        ```

    - GUI App with OpenCV

        ```
        cmake -DCMAKE_GENERATOR_PLATFORM=x64 -DENABLE_OPENCV=TRUE ..
        ```

4. Build and run the app:

    ```
    cmake --build . --config release
    cd release
    mrz license.txt
    ```

    

### Linux
1. Install CMake:

    ```
    sudo apt-get install cmake
    ```

2. Copy `*.so` files to `platform/linux` folder.
3. Create a build folder:
    
    ```
    mkdir build
    cd build
    ```

4. Configure the project.

    - Command-line app

        ```
        cmake ..
        ```

    - GUI App with OpenCV

        ```
        cmake -DENABLE_OPENCV=TRUE ..
        ```

5. Build and run the app:

    ```
    cmake --build . --config release
    ./mrz license.txt
    ```