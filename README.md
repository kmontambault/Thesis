# Reconstruction from Fragments

main.cpp is initially set up for evaluating the algorithm, not for reconstruction. 
The path of a BMP file will be passed as the first command line argument, and the location to store the results will be the second command line argument.
Saving results is optional.

To generate an output image, lines 748 and 749 must be uncommented in main.cpp, and an '/images' folder must be created within the same directory as main.cpp.

The python script will run tests on a set of BMP images all located in the same folder defined on line 6, and will output results to a location defined on line 7.
There is a large portion of commented code in this file, which can be used to filter results once they have been generated.
