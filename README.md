# cmpbin
Command line program that compares files from two directories by comparing binary content of files ("cmpbin" - compare binary). Use-case: when processing and export of thousands of binary files has been changed but filenames of created files are different after each export and it is needed to verify correctness of exported files after change, i.e. that all files are still created as before and that their content hasn't changed (only names are allowed to be different). To perform binary comparison this program uses Murmur3 hash (https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp).     

This project is compiled using MSVC with Unicode defined, contains Windows specific type definitions and API calls.     
      
Example usage:    
     
- run command prompt and type cmpbin <directory 1> <directory 2>
- when program starts it outputs each file(s) from <directory 1> and file(s) from <directory 2> that match by binary content
