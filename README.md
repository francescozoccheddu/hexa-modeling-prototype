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

## Compatibility

Requires [CMake](https://cmake.org/) 3.8+, a modern C++20 compiler and OpenGL 1.1 support.

### Tested on

- MSVC v143 with Windows 11 SDK (10.0.22621.0) on Windows 11 21H2 and Visual Studio 2022 17.3.6 with built-in CMake tools.  
  No setup required.  
  Everything works fine.

- g++ 10.3 on Ubuntu 22.04 (Wayland) with CMake 3.24.2.  
  Make sure to have `libgl1-mesa-dev`, `libglu1-mesa-dev` and `xorg-dev` installed and g++ 10 or latter as the default g++ version (see `update-alternatives`).  
  I think there is a delay in the trigger of the mouse buttons release event and some minor rendering issues. I need to investigate further. Still usable.

- g++ 10.3 on WSL2 (Windows 11 21H2) with Ubuntu 22.04 distro and CMake 3.24.2 or through Visual Studio 2022 17.3.6 with built-in CMake tools for Linux.  
  Follow the instructions for g++ 10.3 on Ubuntu 22.04. Append `export DISPLAY=$(ip route list default | awk '{print $3}'):0` and `export LIBGL_ALWAYS_INDIRECT=1` to your `~/.bashrc` file.  
  There is a problem when swapping the OpenGL buffers. I need to investigate further. Compiles fine but not usable for now.

> **NOTE:**  
> With this configuration Cinolib generates hundreds of warnings (OGDF might also generate some warnings on g++). Compilation should succeed anyway; just ignore them.
