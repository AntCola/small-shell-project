Small Shell Program:

This imitates a shell in a linux environment with three built in commands - 'cd', 'status', and 'exit'. Cd allows for the change of the current working directory with the support of absolute and relative paths. Status will provide the exit status the shell currently holds from the previous process ran. Exit will allow for the exiting of the shell. Apart from these three built in commands, the shell also allows for the use of the exec() family functions. Background and foreground commands are supported. The redirection of the writing and reading of files/commands is also supported. Ctrl-C is used to exit out of any non built in commands currently running. Ctrl-Z toggles if you want to run commands in foreground only mode or allow for background process to be run. To run a background command insert an ampersand symbol as the last element of your command. 


HOW TO COMPILE WITH MAKEFILE:
-use command 'make'
-after using make in the terminal the executable 'smallsh' will be created to run
-to clean up the executables created type 'make clean'

IF ISSUES WITH MAKEFILE:
-in the terminal use 'gcc -o smallsh smallsh.c' to compile the smallsh executable
