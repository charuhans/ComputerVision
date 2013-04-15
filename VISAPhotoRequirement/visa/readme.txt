Automated assessment of VISA Photo requirements
===============================================


List of the files and directories
=================================
   - Source codes: All the source code are in "PassportPhoto" folder:
         "PassportPhoto\Blur" contains the code for blur detection
         "PassportPhoto\Exposure" contains the code for wrong exposure detection
         "PassportPhoto\Orientation" contains the code for wrong orientation detection
         "PassportPhoto\EyeClose" contains the code for eyes closing detection
         "PassportPhoto\MouthClose" contains the code for mouth closing detection
         "PassportPhoto\wait" & "PassportPhoto\combine" contains some ultilities
   - Data: There are 2 datasets:
         "Imgs10" contains the 10 photos
         "Imgs683" contains the 683 photos
         "haarcascades" contains the OpenCV data files for the face detection algorithm
   - Results:
         "output_Imgs10.txt" contains the result for the 10-photos dataset
         "output_Imgs683.txt" contains the result for the 683-photos dataset
   - Scripts: There are some Windows batch files to run our project:
         "run1file.bat" to run assessment for a single photo
	 "run1folder.bat" to run assessment for all photos in one folder
	 "run_Imgs10.bat" to run assessment for 10-photos dataset
	 "run_Imgs683.bat" to run assessment for 683-photos dataset


How to compile and run
======================
   - Requirements: Visual C++ 2010 to compile the source codes
                   Matlab to run the Blur analysis
                   OpenCV 2.2 should be installed in C:\OpenCV2.2
   - Compile the source codes: Open the Visual Studio project files "PassportPhoto\PassportPhoto.sln". 
     Compile the projects in release mode. The OpenCV include directory, include files or library files 
     might need to be re-configurated if the OpenCV is not installed in C:\OpenCV2.2.
   - Run the program: Use the script to run the project:
         Run "run_Imgs10.bat" (without any parameters) to generate the "output_Imgs10.txt"
	 Run "run_Imgs683.bat" (without any parameters) to generate the "output_Imgs683.txt"
	 Run "run1file.bat <Photo> <Output>" to assessment the file <Photo> and save the result to <Output>
	 Run "run1folder.bat <Folder> <Output>" to assessment all files in folder <Folder> and save the result to <Output>
     Notes:
         The working path must be the same directory with batch files.
         The path to matlab must be included in the system environment variables.
