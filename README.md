# cmpbin
CMPBIN - CROSS-PLATFORM GUI DESKTOP APPLICATION WHICH COMPARES FILES IN TWO FOLDERS BY THEIR BINARY CONTENT AND FINDS MATCHES AND UNIQUE FILES

Ivan Golović

This application has two builds, one in form of Debian (.deb) package and other in form of Windows .exe file with .dll dependencies (enclosed in .zip file). These builds are built from same source code and function in the same way on both operating systems, purpose of this program is to compare binary content of all files from two directories and detect:
- files that exist in both directories
- files that exist only in directory 1
- files that exist only in directory 2
Comparison result is displayed in listview and textual version of comparison that can be copy-pasted.

INTRODUCTION

This article describes how to build a C++ application that compares all files in two folders according to their binary content using fast-generated hashes (using Austin Appleby's MurMur3 hash) and dictionaries so that optimal performance is achieved. Emphasis is put on performance and cross-platform capability since the application is using wxWidgets framework to avoid direct calls to OS-specific APIs.
Source code might be useful to developers who have interest in binary comparison of files and cross-platform desktop development using wxWidgets in C++.

Application builds come in two variants:
-	Debian - .dpkg file build which installs Debian package - to install user is required to install .deb file and afterwards run "sudo apt-get install -f" to install dependencies (exact dependencies are listed in release notes since version 3.0.0.0)
-	Windows - .zip file build which includes .exe file and .dll dependencies which need to be extracted in directory

BACKGROUND

I needed this at work when I needed to implement a comprehensive performance improvement of large export functionality and verify that actual file content of produced files hasn't changed (that all bytes in all files are same as before the performance improvement changes to code). Another factor was that file names were randomly created at each export so it wasn't possible to pair "before change" and "after change" files by their names, one could only compare binary content of files each-with-each and conclude that set of "before" and "after" files were same if there was same number of files in both directories and each file in "before" directory could be paired one file in "after" directory according to their binary content.

DEVELOPMENT TOOLS

These were tools used for development of version 3.0.0.0 of cmpbin (in case of later development versions may change):
-	Debian - Code::Blocks 20.03 IDE installed from Debian Bullseye package, using GCC compiler. IDE was configured to use Linux edition of wxWidgets 3.0.5 framework built from source on Debian Bullseye
-	Windows - Visual Studio 2017 IDE, using MSVC compiler. IDE was configured to use Windows edition of wxWidgets 3.1.4 installed from downloaded Windows installer

HOW IT WORKS

User selects directories "directory 1" and "directory 2". After comparison is started program will:
- create hashes of files in "directory 1" and put them in dictionary of file hashes from "directory 1"
- create hashes of files in "directory 2" and put them in dictionary of file hashes from "directory 2"
- loop through hashes of files from "directory 1" and detect matches with file hashes from "directory 2", detect file hashes that exist only in "directory 1" but not in "directory 2"
- loop through hashes of files from "directory 2" and detect file hashes that exist only in "directory 2" but not in "directory 1"
- display hashes and file names matched in both directories and display file names that exist only "directory 1" or "directory 2"
- if user wishes so, user can copy into clipboard textual representation of result and make it available for pasting

POINTS OF INTEREST

-	This software utilises Murmur3 algorithm implementation by Austin Appleby (https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp)
-	OS versions on which this software was tested are Linux Debian Bullseye and Buster, Windows 10
-	Debian version needs to have certain wxWidgets dependencies (packages) installed which can be installed using the usual procedure of installing the applicaiton itself using "sudo dpkg -i name-of-cmpbin-package.deb" and afterwards installing the needed dependencies automatically by running "sudo apt-get install -f"

HISTORY

-	1.0.0.0 - Should be skipped
-	2.0.0.0 - Should be skipped
-	3.0.0.0 - 2021-03-31 - First fully functional version in both supported operating systems (Linux Debian and Windows)

LICENSE

This article, along with any associated source code and files, is licensed under GPL v3 License.

Screenshot while running on Debian:
![screenshot](./screenshot-debian.png?raw=true)

Screenshot while running on Windows 10:
![screenshot](./screenshot.png?raw=true)

