#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void){
    int kill = 1;
    int returnStatus;
    int exitStatus = 0;
    int numberRuns = 0;

    size_t size = 0;
    char* buffer = NULL;
    char* args[512];

    //Set kill = 0 when want to exit out of our shell
    while(kill || numberRuns > 5){

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
        printf("Command: %s\n", token1);

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
                //Need to chdir to what HOME is set to as an env variable
                printf("only cd was passed through\n");
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
            }
        }

        //Exit built in command
        else if (strcmp(token1,"exit") == 0){
            kill = 0;
            printf("Exited with an exit status of: %d\n",exitStatus);
        }

        //Status built in command
        else if (strcmp(token1,"status") == 0){
            printf("Exit status: %d\n",exitStatus);
            fflush(stdout);
        }

        //Need to use exec built in function and fork (command was neither cd, exit, or status)
        else{
            
            //NEED TO FIGURE OUT HOW TO REDIRECT IF THE ARRAY STARTS WITH "<" or ">"
            //ALSO NEED TO SIGNIFY IF BACKGROUND OR FOREGROUND AS WELL
            //ALSO NEED TO SETUP SIGNALS TO CATCH CTRL-Z and CTRL-C


            pid_t spawnPid = -5;
            spawnPid = fork();
            switch(spawnPid){
                case -1:
                    perror("fork() failed!");
                    exit(1);
                    break;
                case 0:    
                    printf("I am the child!\n");
                    fflush(stdout);
                    break;
                default:
                    printf("I am the parent!\n");
                    fflush(stdout);
                    break;
            }
            printf("Both processes done running");
            fflush(stdout);
            kill = 0;
        }

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