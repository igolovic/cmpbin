# cmpbin
Cross-platform GUI application (x64 Windows binaries exist, Linux build is planned) that compares files from two directories by comparing binary content of files ("cmpbin" - compare binary). Use-case: when export functionality of thousands of binary files has been changed but filenames of created files are different in each export (although content is same) and it is needed to verify correctness of exported files after change, i.e. that all files are still created as before and that their content hasn't changed (only names are allowed to be different). Program uses wxWidgets libraries to perform cross-platform access to file system and GUI, for hashing Murmur3 algorithm is used (https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp).     

This project is compiled using MSVC with Unicode strings.     
      
Screenshot while running on Windows:
![screenshot](./screenshot.png?raw=true)
