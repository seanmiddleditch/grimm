Getting Started
===============

Development Environment
-----------------------

We assume Windows 10 environment. Development will use the Microsoft toolchain, aka Visual Studio.

Software
--------

The following software will be presumed to be installed.

**Visual Studio**

- Install [Visual Studio 2019 Community](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&rel=16). Make sure to include the C++ and CMake workloads.

**Git**

- Install [Git for Windows](https://github.com/git-for-windows/git/releases/download/v2.21.0.windows.1/Git-2.21.0-64-bit.exe).

- Install [Git Fork](https://git-fork.com/update/win/ForkInstaller.exe) or another git frontend of your choice. The command line or the one built into Visual Studio will suffice.

SDKs
----

Several binary dependencies currently need to be installed manually.

**SDL2**

- Grab the [SDL2 SDK](https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip) and unpack this file anywhere on your PC.

- Then edit your environment variables and add the path where you unpacked the zip under the `SDL2DIR` variable.

**AssImp**

- Install the [AssImp 4.1 SDK Installer](https://github.com/assimp/assimp/releases/download/v4.1.0/assimp-sdk-4.1.0-setup.exe).

**Doxygen**

- Install the latest [Doxygen](http://www.doxygen.nl/download.html)

Ubuntu in WSL
-------------

- [Enable Windows Subsystem for Linux and install Ubuntu](https://docs.microsoft.com/en-us/windows/wsl/install-win10)

- Open Ubuntu and install required components:
  ```
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo add-apt-repository ppa:janisozaur/cmake-update-bionic
  sudo apt-get update
  sudo apt-get install g++ g++-10 ninja-build cmake
  ```

Building
--------

- Clone the repository `https://github.com/potatoengine/potato`. Make sure you also checkout all submodules and not just the root repository.

- Open the folder you just checked out in Visual Studio. If you have the Visual Studio CMake workload installed, it will automatically configure the build for you.

- After the configuration steps are complete, go to the menu bar at click `Project`->`Build All`.

- To build in Ubuntu on WSL, select the `Linux via WSL (Debug Static)` configuration in Visual Studio, assuming you followed the steps above for setting up Ubuntu in WSL.

Running
-------

You will need to convert the demo resources before running for the first time.

- In the Visual Studio Solution Explorer view, right-click the root of the project folder. There should be menu items at the bottom to Convert Resources. Select the appropriate one for your
build configuration (Debug or Release).

- To run the demo shell, select shell.exe in the dropdown on the toolbar (it says `Select Startup Item` by default). Then hit play.
