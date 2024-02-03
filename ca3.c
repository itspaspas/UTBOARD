#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include "functions.h" 

#define ENTER '\n'
#define QUOTE '\"'

int main(){
    system("color 5F");
    User* users = NULL;
    loadAllData(&users);

    printLogo();
    User* loggedInUser = NULL;
    char* input;
    char* rest;
    char* command;
    char* username;
    char* password;

    while (1) {
        printf("Enter command (signup or login), followed by username and password in quotes if containing spaces, or 'exit' to quit:\n");
        printf("> ");
        input = dynamicInput();
        rest = input;  // Initialize rest to the start of input

        if (input == NULL || strncmp(input, "exit", 4) == 0) {
            free(input);
            break;
        }

        command = getNextToken(&rest);  // Extract command

        if (command && (strcmp(command, "signup") == 0 || strcmp(command, "login") == 0)) {
            username = getNextToken(&rest);  // Extract username
            password = getNextToken(&rest);  // Extract password

            if (username && password) {
                if (strcmp(command, "signup") == 0) {
                    loggedInUser = signupWithArgs(&users, username, password);
                } else if (strcmp(command, "login") == 0) {
                    loggedInUser = loginWithArgs(users, username, password);
                }
            } else {
                printf("Invalid format. Please follow the '<command> \"<username>\" \"<password>\"' format.\n");
            }
        } else {
            printf("Unknown command. Please use 'signup' or 'login'.\n");
        }

        free(input);

        if (loggedInUser) {
            clearScreen();
            boardsMenu(loggedInUser);
            loggedInUser = NULL;
        }
    }

    saveAllData(users);
    freeAllData(&users);
    printf("Exiting the program.\n");
    return 0;
}

