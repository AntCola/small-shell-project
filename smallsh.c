#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

//global flag to determine background process or not (0 = no bg, 1 = bg process)
int bg = 0;

//global flag to determine if foreground only or not based on ctrl-z signal
int foregroundOnly = 0;

//Num background process running
int bgNum = 0;

//array to hold background pids
int bgPids[256]; 

//Iterate through array holding background PID's so we can print complete processes
//at the start of shell prompt as suggested per assignment guidelines
void printBg(){
    int inc = 0;
    int bgExitMethod;

    for(inc; inc < bgNum; inc++){
        if(waitpid(bgPids[inc], &bgExitMethod, WNOHANG) > 0){
            //Normal exit (no signal) print PID along with exit status
            if (WIFEXITED(bgExitMethod)){
                printf("background pid %d is done: exit value %d\n", bgPids[inc], WEXITSTATUS(bgExitMethod));
                fflush(stdout);
            }
            //If terminated via signal report signal that terminated it
            if(WIFSIGNALED(bgExitMethod)){
                printf("background pid %d is done: terminated by signal %d", bgPids[inc], WTERMSIG(bgExitMethod));
                fflush(stdout);
            }
        }
    }
}

//Handles CTRL-Z input so that our foreground only flag can be switched
void sigstp_handler(int signo){
    //Turn foreground only mode on
    if(foregroundOnly == 0){
        foregroundOnly = 1;
        printf("\nEntering foreground-only mode (& is now ignored)\n:");
        fflush(stdout);
    }
    //Turn foreground only mode off
    else if(foregroundOnly == 1){
        foregroundOnly == 0;
        printf("\nExiting foreground-only mode\n:");
        fflush(stdout);
    }
}

int main(void){
    int kill = 1;
    int returnStatus;
    int exitStatus = 0;

    size_t size = 0;
    char* buffer = NULL;
    char* args[512];

    //Set up SIGINT to catch Ctrl-C and ignore with SIG_IGN
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = SIG_IGN;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &SIGINT_action, NULL);

    //Redirect CTRL-Z to handler function to change bg flag between 1 and 0
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = &sigstp_handler;
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    char shellPid[50];
    sprintf(shellPid, "%d", getpid());

    //Set kill = 0 when want to exit out of our shell
    while(kill){
        
        //Print each completed bg process at the start of each prompt
        printBg();

        //Initialize array to NULL to allow for the reuse
        int h = 0;
        for(h; h < 512; h++){
            args[h] = NULL;
        }

        //Do while loop to make sure user enters something
        do{
            printf(":");
            fflush(stdout);

            //User input is held in buffer
            getline(&buffer, &size, stdin);
            //Remove newline character at end of string
            buffer[strcspn(buffer, "\n")] = 0;

        } while(strlen(buffer) < 1);

        //Copy input to be tokenized for arg array
        char* argsCopy;
        argsCopy = malloc(sizeof(char) * (strlen(buffer) + 1));
        strcpy(argsCopy, buffer);

        //Tokenize buffer by spaces to set args array to all arguments for use in exec
        char* argTokens = strtok(argsCopy," ");
        int i = 0;
        while(argTokens != NULL){
            args[i++] = argTokens;
            argTokens = strtok (NULL, " ");
        }
        //Set totalArgs to save number of items given into args for later use
        int totalArgs = i;

        //Check if it needs to be a background process
        if(strcmp(args[totalArgs - 1], "&") == 0){
            bg = 1;
            //After global flag marked, remove & to be ready for exec
            args[totalArgs - 1] = NULL;
            totalArgs--;
        }

        //Replace && with the process ID if present in the command list
        int idCheck = 0;
        for(idCheck; idCheck < totalArgs; idCheck++){
            if(strcmp(args[idCheck], "$$") == 0){
                args[idCheck] = shellPid;
                // printf("$$ replaced with %d", args[idCheck]);
                // fflush(stdout);
            }
        }

        //COMMENT FOR MY SELF REMOVE WHEN DONE
        // if(bg == 1){
        //     printf("Background checked\n");
        //     fflush(stdout);
        // }

        //Tokenize command to be ready for built in commands
        char* token1 = strtok(buffer, " ");

        // // COMMENT OUT THIS IS FOR MY OWN CHECKING)
        // printf("Token1 length: %d\n",strlen(token1));
        // fflush(stdout);
        // printf("Command: %s\n", token1);
        // fflush(stdout);

        //Tokenize buffer so I can check command and destination (MIGHT BE ABLE TO USE ARGS FOR THIS, KEEPING THIS FOR NOW)
        if(strncmp(buffer,"cd",2) == 0){
            //Tokenize second word of command
            char* token2 = strtok(NULL," ");

            //There is no second word
            //Only 'cd' read, chdir to environment variable as HOME
            if(token2 == NULL){
                //Set directory to home directory
                char* homeDir = getenv("HOME");

                //May need to validate this a different way
                chdir(homeDir);

                char* currentDir = getcwd(NULL, 0);

                // // COMMENT OUT THIS IS FOR MY OWN CHECKING)
                // printf("current directory: %s\n", currentDir);
                // fflush(stdout);
                // //Need to chdir to what HOME is set to as an env variable
                // printf("only cd was passed through\n");
                // fflush(stdout);
            }

            //Token 2 holds destination with cd, use token2 to chdir
            else if (strlen(token2) > 0){
                
                // // COMMENT OUT THIS IS FOR MY OWN CHECKING)
                // printf("Destination: %s\n", token2);
                // printf("Token2 length: %d\n", strlen(token2));

                //change to directory desired, it should change directory in this if statement
                if(chdir(token2) == -1){
                    printf("invalid destination\n");
                    fflush(stdout);
                    //Exited due to error
                    exitStatus = 1;
                }
                else{
                    //Ran properly
                    exitStatus = 0;
                }
                //check if directory properly changed (COMMENT OUT THIS IS FOR MY OWN CHECKING)
                char* currentDir = getcwd(NULL, 0);
                printf("current directory: %s\n", currentDir);
                fflush(stdout);
            }
        }

        //Exit built in command
        else if (strcmp(token1,"exit") == 0){
            kill = 0;
            // printf("Exited with an exit status of: %d\n",exitStatus);
            // fflush(stdout);
        }

        //Status built in command
        else if (strcmp(token1,"status") == 0){
            printf("exit value %d\n",exitStatus);
            fflush(stdout);
        }

        //Need to use exec built in function and fork (command was neither cd, exit, or status)
        else{
            
            //NEED TO FIGURE OUT HOW TO SETUP CTRL-Z FOR BOTH PARENT AND CHILD AND NEED TO FIGURE OUT HOW TO MAKE
            //      CTRL-C BEHAVE DIFFERENTLY WITH CHILD


            pid_t spawnPid = -5;
            //Used to determine child exit status
            int childExitMethod = -5;
            spawnPid = fork();
            switch(spawnPid){
                //Error after fork
                case -1:
                    perror("fork() failed!");
                    exit(1);
                    break;

                //Child process
                case 0:    
                    //FOR MY OWN USE DELETE WHEN DONE
                    // printf("I am the child!\n");
                    // fflush(stdout);

                    //Set CTRL-C signal handler to default when called by child (will only kill child process not parent)
                    SIGINT_action.sa_handler = SIG_DFL;
                    sigaction(SIGINT, &SIGINT_action, NULL);

                    //Check if redirection is needed
                    int redirectionNeeded = 0;
                    int inc = 0;
                    char* FILE;
                    int fileDescriptor;

                    //Loop through argument list to check for > or <
                    for(inc; inc < totalArgs; inc++){
                        if(strcmp(args[inc],">") == 0 || strcmp(args[inc],"<") == 0){
                            redirectionNeeded = 1;
                            //Set filename to the next item in array after < or >
                            FILE = strdup(args[inc + 1]);
                            //Need to redirect to /dev/null because background process
                            if(bg == 1){
                                fileDescriptor = open("/dev/null", O_RDONLY);
                                if(dup2(fileDescriptor, STDIN_FILENO) == -1){
                                    printf("cannot redirect command\n");
                                    fflush(stdout);
                                    exit(1);
                                }
                                if(dup2(fileDescriptor, STDOUT_FILENO) == -1){
                                    printf("cannot redirect command\n");
                                    fflush(stdout);
                                    exit(1);
                                }
                            }
                            //Need to redirect input to foreground
                            else{
                                if(strcmp(args[inc], "<") == 0){
                                    fileDescriptor = open(FILE, O_RDONLY);
                                    if(fileDescriptor == -1){
                                        printf("can't open %s for input\n", FILE);
                                        fflush(stdout);
                                        exit(1);
                                    }
                                    if(dup2(fileDescriptor, STDIN_FILENO) ==  -1){
                                        printf("cannot redirect command\n");
                                        fflush(stdout);
                                        exit(1);
                                    }
                                }
                                //Need to redirect output
                                else if(strcmp(args[inc], ">") == 0){
                                    fileDescriptor = open(FILE, O_CREAT | O_RDWR | O_TRUNC, 0644);
                                    if(fileDescriptor == -1){
                                        printf("cannot open %s for output\n", FILE);
                                        fflush(stdout);
                                        exit(1);
                                    }
                                    if(dup2(fileDescriptor, STDOUT_FILENO) == -1){
                                        printf("cannot redirect command\n");
                                        fflush(stdout);
                                        exit(1);
                                    }
                                }
                            }
                        }
                    }

                    //Remove all but commmand to pass into exec if redirection needed
                    if(redirectionNeeded == 1){
                        int p = 1;
                        for(p; p < totalArgs; p++){
                            args[p] = NULL;
                        }
                    }

                    //Entered comment, so we need to ignore
                    if(strncmp(args[0],"#",1) == 0){
                        exit(0);
                    }

                    // //FOR MY OWN CHECKING REMOVE WHEN DONE
                    // if(redirectionNeeded == 1){
                    //     printf("Redirection needed\n");
                    //     fflush(stdout);
                    // }


                    //Run execvp and exit with a status of 1 if failure
                    if (execvp(args[0], args) == -1){
                        printf("Exec Failure!");
                        fflush(stdout);
                        exit(1);
                    }

                    break;

                //Parent process    
                default:
                    // printf("I am the parent!\n");
                    // fflush(stdout);

                    //Background process
                    if(bg == 1 && foregroundOnly == 0){
                        waitpid(spawnPid, &childExitMethod, WNOHANG);
                        bgPids[bgNum] = spawnPid;
                        bgNum++;
                        // printf("Background pid is %d\n", spawnPid);
                        // fflush(stdout);
                    }                
                    else{
                        //Stalls and waits until child terminates
                        waitpid(spawnPid, &childExitMethod, 0);

                        //Set our exit status variable to the the method of exit returned from child process
                        exitStatus = WEXITSTATUS(childExitMethod);
                    
                        //FOR PERSONAL CHECK WILL REMOVE AT END WHEN DONE TESTING
                        // printf("Exit status:%d\n", exitStatus);
                        // fflush(stdout);
                    
                        //If the process was terminated by a signal then we update it this way
                        if(WIFSIGNALED(childExitMethod) != 0){
                            printf("signal %d terminated method\n", WTERMSIG(childExitMethod));
                            fflush(stdout);
                            exitStatus = WTERMSIG(childExitMethod);
                         }
                    }

                    break;
            }
            // //FOR MY OWN USE, DELETE WHEN DONE
            // printf("Both processes done running\n");
            // fflush(stdout);
        }

        //Free contents of argsCopy to allow for the reuse 
        free(argsCopy);

        //Reset bg flag for next run
        bg = 0;
    }

    return 0;   
}