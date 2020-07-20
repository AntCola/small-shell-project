#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void handle_SIGINT(int signo){
    char* message = "\nCaught SIGINT";
    write(STDOUT_FILENO,message,50);
}

int main(void){
    int kill = 1;
    int returnStatus;
    int exitStatus = 0;
    int numberRuns = 0;

    size_t size = 0;
    char* buffer = NULL;
    char* args[512];

    //Set up SIGINT to catch Ctrl-C and save it from killing process
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = handle_SIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &SIGINT_action, NULL);

    //Set kill = 0 when want to exit out of our shell
    while(kill || numberRuns > 5){

        //Initialize array to NULL to allow for the reuse
        int h = 0;
        for(h; h < 512; h++){
            args[h] = NULL;
        }

        printf(":");
        fflush(stdout);

        //User input is held in buffer
        getline(&buffer, &size, stdin);

        //Remove newline character at end of string
        buffer[strcspn(buffer, "\n")] = 0;

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

        //Tokenize command to be ready for built in commands
        char* token1 = strtok(buffer, " ");
        // COMMENT OUT THIS IS FOR MY OWN CHECKING)
        printf("Token1 length: %d\n",strlen(token1));
        fflush(stdout);
        printf("Command: %s\n", token1);
        fflush(stdout);

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

                // COMMENT OUT THIS IS FOR MY OWN CHECKING)
                printf("current directory: %s\n", currentDir);
                fflush(stdout);
                //Need to chdir to what HOME is set to as an env variable
                printf("only cd was passed through\n");
                fflush(stdout);
            }

            //Token 2 holds destination with cd, use token2 to chdir
            else if (strlen(token2) > 0){
                
                // COMMENT OUT THIS IS FOR MY OWN CHECKING)
                printf("Destination: %s\n", token2);
                printf("Token2 length: %d\n", strlen(token2));

                //change to directory desired, it should change directory in this if statement
                if(chdir(token2) == -1){
                    printf("Invalid destination.\n");
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
            printf("Exited with an exit status of: %d\n",exitStatus);
            fflush(stdout);
        }

        //Status built in command
        else if (strcmp(token1,"status") == 0){
            printf("Exit status: %d\n",exitStatus);
            fflush(stdout);
        }

        //Need to use exec built in function and fork (command was neither cd, exit, or status)
        else{
            
            //NEED TO FIGURE OUT HOW TO SIGNIFY IF BACKGROUND OR FOREGROUND AND RUN BACKGROUND/FOREGROUND 
            //     MY THOUGHTS HERE ARE WE CHECK FOR THE '&' AND IF THAT'S PRESENT WE ADD A FLAG FOR BG vs FG
            //     IF IT'S BG WE NEED TO FORK OFF ANOTHER PROCESS TO RUN IN BACKGROUND? STUCK ON THIS
            //NEED TO FIGURE OUT HOW TO SETUP CTRL-Z FOR BOTH PARENT AND CHILD AND NEED TO FIGURE OUT HOW TO MAKE
            //      CTRL-C BEHAVE DIFFERENTLY WITH CHILD

            pid_t spawnPid = -5;
            //Used to determine child exit status
            int childExitMethod = -5;
            spawnPid = fork();
            switch(spawnPid){
                case -1:
                    perror("fork() failed!");
                    exit(1);
                    break;
                case 0:    
                    printf("I am the child!\n");
                    fflush(stdout);

                    //Set CTRL-C signal handler to default when called by child (will only kill child process not parent)
                    SIGINT_action.sa_handler = SIG_DFL;
                    sigaction(SIGINT, &SIGINT_action, NULL);

                    //Entered comment, so we need to ignore
                    if(strcmp(args[0],"#") == 0){
                        exit(0);
                    }

                    //NEED TO SET UP REDIRECTION HERE

                    //Error calling exec so an exit status of 1
                    if (execvp(args[0], args) == -1){
                        printf("Exec Failure!");
                        fflush(stdout);
                        exit(1);
                    }

                    break;
                default:
                    printf("I am the parent!\n");
                    fflush(stdout);

                    //Stalls and waits until child terminates
                    waitpid(spawnPid, &childExitMethod, 0);

                    //Set our exit status variable to the the method of exit returned from child process
                    exitStatus = WEXITSTATUS(childExitMethod);
                    
                    //FOR PERSONAL CHECK WILL REMOVE AT END WHEN DONE TESTING
                    printf("Exit status:%d\n", exitStatus);
                    fflush(stdout);
                    
                    //If the process was terminated by a signal then we update it this way
                    if(WIFSIGNALED(childExitMethod) != 0){
                        printf("signal %d terminated method\n", WTERMSIG(childExitMethod));
                        fflush(stdout);
                        exitStatus = WTERMSIG(childExitMethod);
                    }
                    break;
            }
            printf("Both processes done running\n");
            fflush(stdout);
            // kill = 0;
        }

        //Free contents of argsCopy to allow for the reuse 
        free(argsCopy);

        //Loop can only run 5 times (trying to save server)
        numberRuns++;
    }

    // //Commands made by me recognized from user input --> now do work with these
    // if(strcmp(buffer, "cd")==0){
    //     printf("cd was found\n");
    // } else if(strcmp(buffer, "exit") == 0){
    //     printf("exit was found\n");
    // } else if(strcmp(buffer, "status") == 0){
    //     printf("status was found\n");
    // } else{
    //     printf("nothing found\n");
    // }




    // int i;
    // for(i = 0; i < strlen(buffer);i++){
    //     printf("%i", i);
    // }

    // printf("%s \n", buffer);
    return 0;   
}