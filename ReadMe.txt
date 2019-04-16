
A project, which will simulate the process scheduling part of an operating system. Time-based scheduling, 
ignoring almost every other aspect of the OS is implemented in this project. Also, we will be using message queues for synchronization.

By Syed Tariq Rashid.



Command Line arguments :

-h : HELP.   
-l : The log file Name provided by the user. If nothing is provided by the user, "default" will be the filename.
-t :  maximum time of the program allowed to run in seconds.If nothing is provided by the user, 2 Sec will be the default time.


In order to run this program..


First execute this script -->  make -f Makefile
After this execute this script --> ./oss -l logfile -t programtime
If you want to remove output files execute this script --> make clean

The output result is stored in the logfile.