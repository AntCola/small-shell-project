#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(void){
    size_t size = 0;
    char* buffer = NULL;

    printf(":\n");
    fflush(stdout);

    //User input is held in buffer
    getline(&buffer, &size, stdin);

    //Remove newline character at end of string
    buffer[strcspn(buffer, "\n")] = 0;

    //Tokenize command 
    char* token1 = strtok(buffer, " ");
    printf("Token1 length: %d\n",strlen(token1));
    printf("Command: %s\n", token1);

    //need to tokenize buffer so I can check command and destination
    if(strncmp(buffer,"cd",2) == 0){
        //Tokenize second word of command
        char* token2 = strtok(NULL," ");

        //There is no second word
        //Only 'cd' read, chdir to environment variable as HOME
        if(token2 == NULL){
            //Need to chdir to what HOME is set to as an env variable
            printf("only cd was passed through\n");
        }

        //Token 2 holds destination with cd, use token2 to chdir
        else if (strlen(token2) > 0){

            printf("Destination: %s\n", token2);
            printf("Token2 length: %d\n", strlen(token2));

            //This means there's a 3rd command (this might be unecessary since skipping if & is present anyways)
            if(token2[strlen(token2 - 1) == ' ']){
                char* token3 = strtok(NULL, "\0");
                //No 3rd command (looking for a &, but we don't need to do anything if present)
                if(token3 == NULL){
                    printf("No 3rd command\n");
                }
                else{
                    printf("3rd command present\n");
                    printf("Mod: %s", token3);
                    printf("Token3 length: %d\n", strlen(token3));
                }
            }
        }
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