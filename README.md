Screenshot while running on Debian:   
![screenshot](./screenshot-debian.png?raw=true)   
    
Screenshot while running on Windows 10:    
![screenshot](./screenshot.png?raw=true)    
   
# cmpbin
CMPBIN - CROSS-PLATFORM GUI DESKTOP APPLICATION WHICH COMPARES FILES IN TWO FOLDERS BY FILE SIZE AND BINARY CONTENT AND FINDS MATCHES AND UNIQUE FILES   
   
Purpose of this application is to compare file size and binary content (to resolve compare files of same size) of all files from two directories and detect:  
    - files of unique size in directory 1 (for which there is no file of same size in directory 2) and conclude it is unique  
    - files of unique size in directory 1 (for which there is no file of same size in directory 2) and conclude it is unique  
    - files that exist in both directories and have same size, for files in directories that have same size compare their binary content using hashes and detect:  
        - unique files in directory 1  
        - unique files in directory 2  
        - files that match by hash in both directories  
Comparison result is displayed in listview and CSV version of comparison result can be copy-pasted.  
Application uses wxWidgets libraries to perform cross-platform access to file system and GUI. For hashing is used Murmur3 algorithm. Application has two builds, one in form of Debian package targeting Linux Debian and other in form of Windows .exe with .dll dependencies targeting Windows. These builds are built from same source code and work identicaly on both operating systems. 
   
## INTRODUCTION
This article describes how to build a C++ application that compares all files in two folders according to their size and binary content by using hashes (Austin Appleby's MurMur3 hash). Dictionaries are used so that better performance is achieved. Also, a pre-comparison by file size is used to minimize number of files for which hash comparison needs to be performed. Emphasis is put on cross-platform capability since the application is using wxWidgets framework to avoid direct calls to OS-specific APIs.
Source code might be useful to developers who have interest in file comparison of files and cross-platform desktop development using wxWidgets in C++.

Application builds come in two variants:
-	Debian - .dpkg file build which installs Debian package - to install, user is required to install .deb file and afterwards run "sudo apt-get install -f" to install dependencies (exact commands and dependencies are listed in release notes since version 3.0.0.0)
-	Windows - .zip file build which includes .exe file and .dll dependencies which need to be extracted in directory and .exe can be run after that

## BACKGROUND
I needed this at work when I needed to implement a comprehensive performance improvement of large export functionality and verify that actual file content of produced files hasn't changed (that all bytes in all files are same as before the performance improvement changes to code). Another factor was that file names were randomly created at each export so it wasn't possible to pair "before change" and "after change" files by their names, one could only compare binary content of files each-with-each and conclude that set of "before" and "after" files were same if there was same number of files in both directories and each file in "before" directory could be paired one file in "after" directory according to their binary content.

## DEVELOPMENT TOOLS
These were the tools used for development of version 3.0.0.0 of cmpbin (in case of later development versions may change):
-	Debian - Code::Blocks 20.03 IDE built form sources to avoid version mismatches with wxWidgets library. Compiler was GCC. IDE was configured to use Linux edition of wxWidgets 3.0.5 framework also built from source.
-	Windows - Visual Studio 2017 IDE,. Compiler was MSVC. IDE was configured to use Windows edition of wxWidgets 3.1.4 installed from downloaded Windows installer.

## HOW IT WORKS
User selects directories "directory 1" and "directory 2". Comparison is started in new "worker" thread and it will:
- enumerate files in both directories and collect file sizes
- compare which files are unique in directories by their file size and which have matching files sizes
- for files that have no matching file size in other folder it can immediatelly determined that they are unique
- for files that have matching file size in both folder, hashes will be generated to determine difference of equality
- create hashes of files in "directory 1" and put them in dictionary of file hashes from "directory 1"
- create hashes of files in "directory 2" and put them in dictionary of file hashes from "directory 2"
- loop through hashes of files from "directory 1" and detect matches with file hashes from "directory 2", detect file hashes that exist only in "directory 1" but not in "directory 2"
- loop through hashes of files from "directory 2" and detect file hashes that exist only in "directory 2" but not in "directory 1"
- for every matched file size join information about file's file size
- create collection of "ListDataItem" items where each item contains file size, hashe, file names matched in directory 1, file names matched in directory 2
- items are added to collection of "ListDataItem" in following order: unique files in directory 1, matched files, unique files in directory 2
- send pointer to collection of "ListDataItem" to main thread through event which denotes end of processing in "worker" thread
- display items from collection of "ListDataItem" in application's listview
- if user wishes so, user can copy into clipboard textual representation of comparison result in form of CSV and make it available for pasting

## POINTS OF INTEREST
-	This software utilises Murmur3 algorithm implementation by Austin Appleby (https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp)
-	OS versions on which this software was tested are Linux Debian Bullseye and Buster, Windows 10
-	on Debian CodeBlocks IDE and wxWidgets were installed (using usual autotools) to avoid any version mismatches between the two
-	installation of cmpbin on Debian needs to have certain wxWidgets dependencies (packages) installed which. Commands to install program itself along with the dependencies is this:
    - sudo dpkg -i current_cmpbin_package_name.deb
    - sudo apt-get install -f

## HISTORY
-	1.0.0.0 - Should be skipped
-	2.0.0.0 - Should be skipped
-	3.0.0.0 - 2021-03-31 - First fully functional version in both supported operating systems (Linux Debian and Windows)
-	4.0.0.0 - processing moved to separate "worker" thread, pre-comparison using file sizes added
-	4.0.1.0 - various improvements and bug fixes of previous version
     
## LICENSE
This article, along with any associated source code and files, is licensed under GPL v3 License.
