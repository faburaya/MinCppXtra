# MinCppXtra

This library gathers some bare minimal additions to C++ standard library:

* Generation of messages for Win32 API error codes.
* Translation of Win32 (SEH) exceptions to C++ exceptions.
	* It requires enabling /EHa in msvc compiler.
* An exception type that provides call stack trace
	* It requires the app debug symbols available.

They are not intended to extend STL or follow its style, but they are easy to use.
The set of features is small, but it normally suffices for developing applications in Windows platform.
Everything is contained in namespace `mincpp`, and the build process of the main project generates an `install` directory with the necessary files (include/ & lib/) to use this library.

(I intend to evolve this code ir order to replace [3fd](https://github.com/faburaya/3fd),
which is a much bigger project with several modules, most of which are never actually
used nowadays.)
