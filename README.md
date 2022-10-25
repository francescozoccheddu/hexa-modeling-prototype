# Hexa Modeling Prototype

## Setup

1. Clone this repository and all its submodules:

```Shell
git clone --recurse-submodules https://github.com/francescozoccheddu/hexa-modeling-prototype
cd hexa-modeling-prototype
```

2. Generate the build system:

```Shell
mkdir out
cd out
cmake ..
```

3. Build:

```Shell
cmake --build .
```

4. Run the `gui` executable.

5. Enjoy! 😉

### Compatibility

Requires [CMake](https://cmake.org/) 3.8+, a modern C++20 compiler and OpenGL 1.1 support.

Tested on MSVC v143 on Windows 11 and g++ 10 and Clang 14 on Ubuntu 22.10. 

> **NOTE:**  
> With this configuration Cinolib generates hundreds of warnings (OGDF might also generate some warnings on g++). Compilation should succeed anyway; just ignore them.

> **NOTE:**  
> On Ubuntu, make sure to have `libgl1-mesa-dev`, `libglu1-mesa-dev` and `xorg-dev` installed.

> **NOTE:**  
> On Ubuntu there is a delay in the trigger of the GLFW mouse buttons release event, so the camera movement feels sticky.