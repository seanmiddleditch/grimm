Using Tracy
===========

Building
--------

First, build the Potato application. This will fetch all dependencies, include Tracy itself.

Second, the dependencies of Tracy must be installed using the vcpkg tool.

Find the Tracy sources at `<root>/`, where `<root>` is the root folder of Potato.

Open a command shell at the root of the Potato source directory.

In the command shell, run `cd deps/tracy-srcvcpkg ; install_vcpkg_dependencies.bat`.

Now open the solution file `deps/tracy-src/profiler/build/win32/Tracy.sln`.

Select the Release build configuration and build the profiler.

Using the Profiler
------------------

Open the Tracy profiler application built in the previous step.

Start the Potato Shell application.

Inside the Tracy profiler, click Connect.

That's it.

See the Tracy documentation at https://github.com/wolfpld/tracy for more information on using Tracy.

Annotating the Client
---------------------

Include the Tracy header file, `#include <Tracy.hpp>`.

The most interesting macros are likely to be `ZoneScoped`, `ZoneScopedN`, and `TracyPlot`.

`ZoneScoped;` will create a "zone" in Tracy, with no name, that lives for the rest of the current scope.

`ZoneScopedN("Some Name");` will create a zone in Tracy with the provided name that lives for the rest of the current scope.

`TracyPlot("Name", value);` will plot a value using the given name.
