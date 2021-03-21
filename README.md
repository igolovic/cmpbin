# cmpbin
Cross-platform GUI program which compares files in two folders by their binary content and finds matches and unique files (Cmpbin - compare binary).  
  
Possible usage example of this program in computer programming: when export functionality of thousands of binary files has been changed but filenames of created files are different in each export (although content is same) and it is needed to verify correctness of exported files after change, i.e. that all files are still created as before and that their content hasn't changed (only names are allowed to be different).  

Program uses wxWidgets libraries to perform cross-platform access to file system and GUI.  
For hashing is used Murmur3 algorithm (https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp).     

Project was built and tested:  
- on Debian Buster using Code::Blocks with GCC and version of wxWidgets for Linux Debian  
- on Windows 10 using Visual Studio 2017 with MSVC and version of wxWidgets for Windows     

Screenshot while running on Debian:
![screenshot](./screenshot-debian.png?raw=true)

Screenshot while running on Windows 10:
![screenshot](./screenshot.png?raw=true)
