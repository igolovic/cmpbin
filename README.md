Program to compare files in two directories by comparing binary content. Use-case: when large processing and export of thousands of binary files has been changed but filenames of created files are different after each export and it is needed to verify correctness of exported files, i.e. that all files were created and that their content hasn't changed (only names are different). To perform binary comparison this program uses Murmur3 hash. This project is compiled using MSVC with Unicode defined, contains Windows specific type definitions and API calls.
      
Example usage:    
     
- run command prompt and type cmpbin <directory 1> <directory 2>
- when program starts it outputs each file(s) from <directory 1> and file(s) from <directory 2> that match by binary content
