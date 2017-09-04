# HrdDataCacheSystem
Generic caching library for C++ with a support for VTK toolkit

This caching library is written in C++ 11 and based on the memoization approach which can be used to speed up applications where some repetition of time consuming processes is occurring.
The library is very generic so it can be used in quite any C++ application but the original motivation for developing such library came from the VTK and its problems with unnecessary computation repetition.

To make the usage in VTK easier, a wrapper was created which allows the user to create a caching version of an existing VTK filter by inheritance and by defining a few predeclared methods.
