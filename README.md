# cmpbin
Cross-platform GUI application that compares files from two directories by comparing binary content of files ("cmpbin" - compare binary).  
  
Possible usage example of this program in computer programming: when export functionality of thousands of binary files has been changed but filenames of created files are different in each export (although content is same) and it is needed to verify correctness of exported files after change, i.e. that all files are still created as before and that their content hasn't changed (only names are allowed to be different).  

Program uses wxWidgets libraries to perform cross-platform access to file system and GUI.  
For hashing is used Murmur3 algorithm (https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp).     

Available Windows x64 binary was compiled using MSVC. Linux Debian build is planned.     
      
Screenshot while running on Windows:
![screenshot](./screenshot.png?raw=true)
