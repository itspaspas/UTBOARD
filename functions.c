#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define ENTER '\n'
#define QUOTE '\"'

typedef struct Task {
    char* name;
    char* priority;
    char* date;
    long id;
    struct Task* next;
} Task;

typedef struct List {
    char* name;
    long id;
    struct List* next;
    Task* tasks;
} List;

typedef struct Board {
    char* name;
    long id;
    struct Board* next;
    List* lists;
} Board;

typedef struct User {
    char* username;
    char* password;
    struct User* next;
    Board* boards;
} User;

typedef struct {
    Task* task;
    char* boardName;
    char* listName;
} TaskInfo;

// Function prototypes
int isValidDate(char* date);
void saveAllData(const User* users);
void saveUsers(const User* user);
void saveBoards(const User* user);
void saveLists(const User* users);
void saveTasks(const User* users);
char* dynamicFgets(FILE* stream);
char** parseCSVLine(char* line, int* fieldCount);
char* dynamicInput();
void loadAllData(User** users);
void loadUsers(User** users);
void loadBoards(User* user);
void loadLists(Board* board);
void loadTasks(List* list);
void freeAllData(User** users);
void freeUsers(User* user);
void freeBoards(Board* board);
void freeLists(List* list);
void freeTasks(Task* task);
void boardsMenu(User* user);
void deleteBoard(User* user);
void createBoard(User* user);
void displayBoards(const User* user);
Board* selectBoard(User* user);
User* loginWithArgs(User* users, const char* username, const char* password);
User* signupWithArgs(User** users, const char* username, const char* password);
char* getNextToken(char** input);
int userExists(User* users, const char* username);
void displayLists(const Board* board);
List* selectList(Board* board);
void createList(Board* board);
void deleteList(Board* board);
void listsMenu(User* user, Board* board);
void displayTasks(const List* list);
Task* selectTask(List* list);
void addTask(List* list);
void editTask(List* list);
void deleteTask(List* list);
void moveTask(Board* board, List* currentList);
void tasksMenu(User* user, Board* board, List* list);
void clearScreen();
int taskDeadlineComparator(const void* a, const void* b);
char* getCurrentDate();
void showUpcomingTasks(User* user);
int compareTasksByPriority(const void* a, const void* b);
int compareTasksByDate(const void* a, const void* b);
void sortTasks(List* list, int (*compFunc)(const void*, const void*));
void sortTasksMenu(List* list);
void printLogo();
long generateUniqueId();

// Helper function to parse a CSV line into fields, considering quoted fields with commas

char** parseCSVLine(char* line, int* fieldCount) {
    int capacity = 10; // Initial capacity for the number of fields
    char** fields = (char**)malloc(capacity * sizeof(char*));
    if (fields == NULL) {
        perror("Memory allocation failed for fields");
        *fieldCount = 0;
        return NULL;
    }

    *fieldCount = 0;
    char* field = line;
    int inQuotes = 0;
    int fieldStart = 1; // Flag to indicate the start of a new field

    for (char* p = line; *p; p++) {
        // Check for quote character
        if (*p == QUOTE) {
            inQuotes = !inQuotes;
            if (fieldStart) {
                field = p + 1; // Start after the quote
            } else if (!inQuotes) {
                *p = '\0'; // End before the quote
            }
            continue; // Skip the quote character
        }

        // Check for comma separator if not within quotes
        if (*p == ',' && !inQuotes) {
            *p = '\0'; // Null-terminate the current field
            if (*fieldCount >= capacity) {
                // Resize the fields array if necessary
                capacity *= 2;
                char** tempFields = (char**)realloc(fields, capacity * sizeof(char*));
                if (tempFields == NULL) {
                    perror("Memory reallocation failed for fields");
                    free(fields);
                    *fieldCount = 0;
                    return NULL;
                }
                fields = tempFields;
            }
            fields[(*fieldCount)++] = field;
            fieldStart = 1;
            field = p + 1; // Start of the next field
        } else if (fieldStart) {
            field = p; // Start of a new field
            fieldStart = 0;
        }
    }

    // Capture the last field if the line doesn't end with a comma
    if (!fieldStart) {
        if (*fieldCount >= capacity) {
            // Resize the fields array if necessary, adding space for one more field
            capacity += 1;
            char** tempFields = (char**)realloc(fields, capacity * sizeof(char*));
            if (tempFields == NULL) {
                perror("Memory reallocation failed for fields");
                free(fields);
                *fieldCount = 0;
                return NULL;
            }
            fields = tempFields;
        }
        fields[(*fieldCount)++] = field;
    }

    return fields;
}

long generateUniqueId() {
    static long uniqueId = 0;
    return ++uniqueId;
}

void clearScreen() {
    Sleep(1000);
    system("cls");
    printLogo();
}

char* dynamicInput() {
    char* input = NULL;
    int ch;
    size_t size = 0;
    size_t length = 0;

    // Read characters until ENTER or EOF is encountered
    while ((ch = getchar()) != ENTER && ch != EOF) {
        // Check if we need to expand the buffer
        if (length + 1 >= size) { // +1 for the null terminator
            size_t newSize = size == 0 ? 2 : size * 2; // Double the buffer size
            char* tempInput = realloc(input, newSize);
            if (!tempInput) {
                free(input); // Free the previously allocated memory
                return NULL; // realloc failed, return NULL
            }
            input = tempInput;
            size = newSize;
        }
        input[length++] = (char)ch;
    }

    // Null-terminate the input string
    if (input != NULL) {
        // Ensure there is space for the null terminator
        if (length >= size) {
            char* tempInput = realloc(input, length + 1); // +1 for the null terminator
            if (!tempInput) {
                free(input);
                return NULL;
            }
            input = tempInput;
        }
        input[length] = '\0';
    }

    return input;
}

char* dynamicFgets(FILE* stream) {
    char* line = NULL;
    size_t size = 0;
    int ch;
    size_t length = 0;

    while ((ch = fgetc(stream)) != EOF && ch != '\n') {
        // Dynamically allocate memory for the line buffer
        if (length + 1 >= size) {
            size = size == 0 ? 1 : size * 2; // Start with 128 bytes and double when needed
            char* new_line = realloc(line, size);
            if (!new_line) {
                free(line);
                return NULL; // Memory allocation failed
            }
            line = new_line;
        }
        line[length++] = (char)ch; // Append character to the line
    }
    if (length == 0 && ch == EOF) {
        free(line);
        return NULL; // No input read
    }
    line[length] = '\0'; // Null-terminate the line
    return line;
}

void saveAllData(const User* users) {
    saveUsers(users);
    saveBoards(users); 
    saveLists(users);  
    saveTasks(users);
}

void saveUsers(const User* users) {
    FILE* fpUsers = fopen("users.csv", "w");
    if (fpUsers == NULL) {
        perror("Unable to open users file for writing");
        return;
    }
    // Write header
    fprintf(fpUsers, "\"Username\",\"Password\"\n");
    // Iterate over all users and write their data to the file
    while (users != NULL) {
        fprintf(fpUsers, "\"%s\",\"%s\"\n", users->username, users->password);
        users = users->next;
    }
    fclose(fpUsers);
}

void saveBoards(const User* users) {
    FILE* fpBoards = fopen("boards.csv", "w");
    if (fpBoards == NULL) {
        perror("Unable to open boards file for writing");
        return;
    }
    // Write header
    fprintf(fpBoards, "\"Board ID\",\"Board Name\",\"Username\"\n");
    // Iterate over all users and their boards and write them to the file
    while (users != NULL) {
        const Board* board = users->boards;
        while (board != NULL) {
            fprintf(fpBoards, "\"%ld\",\"%s\",\"%s\"\n", board->id, board->name, users->username);
            board = board->next;
        }
        users = users->next;
    }
    fclose(fpBoards);
}

void saveLists(const User* users) {
    FILE* fpLists = fopen("lists.csv", "w");
    if (fpLists == NULL) {
        perror("Unable to open lists file for writing");
        return;
    }
    // Write header
    fprintf(fpLists, "\"List ID\",\"List Name\",\"Board ID\"\n");
    // Iterate over all users, their boards, and lists, and write them to the file
    while (users != NULL) {
        const Board* board = users->boards;
        while (board != NULL) {
            const List* list = board->lists;
            while (list != NULL) {
                fprintf(fpLists, "\"%ld\",\"%s\",\"%ld\"\n", list->id, list->name, board->id);
                list = list->next;
            }
            board = board->next;
        }
        users = users->next;
    }
    fclose(fpLists);
}

void saveTasks(const User* users) {
    FILE* fpTasks = fopen("tasks.csv", "w");
    if (fpTasks == NULL) {
        perror("Unable to open tasks file for writing");
        return;
    }
    // Write header
    fprintf(fpTasks, "\"Task ID\",\"Task Name\",\"Priority\",\"Date\",\"List ID\"\n");
    // Iterate over all users, their boards, lists, and tasks, and write them to the file
    while (users != NULL) {
        const Board* board = users->boards;
        while (board != NULL) {
            const List* list = board->lists;
            while (list != NULL) {
                const Task* task = list->tasks;
                while (task != NULL) {
                    fprintf(fpTasks, "\"%ld\",\"%s\",\"%s\",\"%s\",\"%ld\"\n",
                            task->id, task->name, task->priority, task->date, list->id);
                    task = task->next;
                }
                list = list->next;
            }
            board = board->next;
        }
        users = users->next;
    }
    fclose(fpTasks);
}

void loadUsers(User** users) {
    FILE* fpUsers = fopen("users.csv", "r");
    if (fpUsers == NULL) {
        perror("Unable to open users file for reading");
        return;
    }

    // Skip the header line
    char* line = dynamicFgets(fpUsers);
    free(line); // Free the header line

    while ((line = dynamicFgets(fpUsers)) != NULL) {
        int fieldCount = 0;
        char** fields = parseCSVLine(line, &fieldCount);

        if (fieldCount >= 2) {
            User* newUser = (User*)malloc(sizeof(User));
            if (newUser == NULL) {
                perror("Memory allocation failed for newUser");
                break; // Exit the loop if memory allocation fails
            }
            newUser->username = strdup(fields[0]);
            newUser->password = strdup(fields[1]);
            newUser->boards = NULL; // Initialize boards to NULL
            newUser->next = *users; // Link the new user to the head of the list
            *users = newUser;       // Update the head of the list to the new user
        } else {
            // Handle the case where the expected number of fields is not met
            fprintf(stderr, "Invalid record format in users.csv: %s", line);
        }

        // Free the dynamically allocated fields
        
        free(line);
    }
    fclose(fpUsers);
}

void loadBoards(User* user) {
    FILE* fpBoards = fopen("boards.csv", "r");
    if (fpBoards == NULL) {
        perror("Unable to open boards file for reading");
        return;
    }

    // Skip the header line
    char* line = dynamicFgets(fpBoards);
    free(line); // Free the header line

    while ((line = dynamicFgets(fpBoards)) != NULL) {
        int fieldCount = 0;
        char** fields = parseCSVLine(line, &fieldCount);

        if (fieldCount >= 3) {
            // Check if the username matches
            if (strcmp(user->username, fields[2]) == 0) {
                Board* newBoard = (Board*)malloc(sizeof(Board));
                if (newBoard == NULL) {
                    perror("Memory allocation failed for newBoard");
                    break;
                }
                newBoard->id = strtol(fields[0], NULL, 10);
                newBoard->name = strdup(fields[1]);
                newBoard->lists = NULL;
                newBoard->next = user->boards;
                user->boards = newBoard;
            }
        } else {
            fprintf(stderr, "Invalid record format in boards.csv: %s", line);
        }

        // Free the dynamically allocated fields
        
        free(line);
    }
    fclose(fpBoards);
}

void loadLists(Board* board) {
    FILE* fpLists = fopen("lists.csv", "r");
    if (fpLists == NULL) {
        perror("Unable to open lists file for reading");
        return;
    }

    // Skip the header line
    char* line = dynamicFgets(fpLists);
    free(line); // Free the header line

    while ((line = dynamicFgets(fpLists)) != NULL) {
        int fieldCount = 0;
        char** fields = parseCSVLine(line, &fieldCount);

        if (fieldCount >= 3) {
            // Check if the board ID matches
            long boardId = strtol(fields[2], NULL, 10);
            if (board->id == boardId) {
                List* newList = (List*)malloc(sizeof(List));
                if (newList == NULL) {
                    perror("Memory allocation failed for newList");
                    break;
                }
                newList->id = strtol(fields[0], NULL, 10);
                newList->name = strdup(fields[1]);
                newList->tasks = NULL;
                newList->next = board->lists;
                board->lists = newList;
            }
        } else {
            fprintf(stderr, "Invalid record format in lists.csv: %s", line);
        }

        // Free the dynamically allocated fields
        
        free(line);
    }
    fclose(fpLists);
}

void loadTasks(List* list) {
    FILE* fpTasks = fopen("tasks.csv", "r");
    if (fpTasks == NULL) {
        perror("Unable to open tasks file for reading");
        return;
    }

    // Skip the header line
    char* line = dynamicFgets(fpTasks);
    free(line); // Free the header line

    while ((line = dynamicFgets(fpTasks)) != NULL) {
        int fieldCount = 0;
        char** fields = parseCSVLine(line, &fieldCount);

        if (fieldCount >= 5) {
            // Check if the list ID matches
            long listId = strtol(fields[4], NULL, 10);
            if (list->id == listId) {
                Task* newTask = (Task*)malloc(sizeof(Task));
                if (newTask == NULL) {
                    perror("Memory allocation failed for newTask");
                    break;
                }
                newTask->id = strtol(fields[0], NULL, 10);
                newTask->name = strdup(fields[1]);
                newTask->priority = strdup(fields[2]);
                newTask->date = strdup(fields[3]);
                newTask->next = list->tasks;
                list->tasks = newTask;
            }
        } else {
            fprintf(stderr, "Invalid record format in tasks.csv: %s", line);
        }

        // Free the dynamically allocated fields
        
        free(line);
    }
    fclose(fpTasks);
}

void loadAllData(User** users) {
    // Load users
    loadUsers(users);

    // Load boards for each user
    for (User* currentUser = *users; currentUser != NULL; currentUser = currentUser->next) {
        loadBoards(currentUser);
    }

    // Load lists for each board of each user
    for (User* currentUser = *users; currentUser != NULL; currentUser = currentUser->next) {
        for (Board* currentBoard = currentUser->boards; currentBoard != NULL; currentBoard = currentBoard->next) {
            loadLists(currentBoard);
        }
    }

    // Load tasks for each list of each board of each user
    for (User* currentUser = *users; currentUser != NULL; currentUser = currentUser->next) {
        for (Board* currentBoard = currentUser->boards; currentBoard != NULL; currentBoard = currentBoard->next) {
            for (List* currentList = currentBoard->lists; currentList != NULL; currentList = currentList->next) {
                loadTasks(currentList);
            }
        }
    }
}

void freeTasks(Task* task) {
    while (task != NULL) {
        Task* currentTask = task;
        task = task->next; // Move to the next task before freeing the current one
        free(currentTask->name);
        free(currentTask->priority);
        free(currentTask->date);
        free(currentTask); // Free the task structure itself
    }
}

void freeLists(List* list) {
    while (list != NULL) {
        List* currentList = list;
        list = list->next; // Move to the next list before freeing the current one
        freeTasks(currentList->tasks); // Free all tasks in the list
        free(currentList->name);
        free(currentList); // Free the list structure itself
    }
}

void freeBoards(Board* board) {
    while (board != NULL) {
        Board* currentBoard = board;
        board = board->next; // Move to the next board before freeing the current one
        freeLists(currentBoard->lists); // Free all lists in the board
        free(currentBoard->name);
        free(currentBoard); // Free the board structure itself
    }
}

void freeUsers(User* user) {
    while (user != NULL) {
        User* currentUser = user;
        user = user->next; // Move to the next user before freeing the current one
        freeBoards(currentUser->boards); // Free all boards for the user
        free(currentUser); // Free the user structure itself
    }
}

void freeAllData(User** users) {
    if (users != NULL) {
        freeUsers(*users); // Free all users and their associated data
        *users = NULL; // Set the users list head to NULL
    }
}

// Function to check if a username already exists in the list of users
int userExists(User* users, const char* username) {
    while (users != NULL) {
        if (strcmp(users->username, username) == 0) {
            return 1; // User exists
        }
        users = users->next;
    }
    return 0; // User does not exist
}

// Function to handle user signup with command line arguments
User* signupWithArgs(User** users, const char* username, const char* password) {
    if (userExists(*users, username)) {
        printf("Username is already taken.\n");
        return NULL;
    }

    User* newUser = malloc(sizeof(User));
    if (!newUser) {
        printf("Failed to allocate memory for new user.\n");
        return NULL;
    }

    newUser->username = strdup(username);
    newUser->password = strdup(password);
    newUser->boards = NULL; // Initialize boards to NULL

    if (!newUser->username || !newUser->password) {
        printf("Failed to duplicate username or password.\n");
        free(newUser->username); // Safe to call free on NULL
        free(newUser->password);
        free(newUser);
        return NULL;
    }

    newUser->next = *users;
    *users = newUser;

    printf("Signup successful.\n");
    return newUser;
}

// Function to handle user login with command line arguments
User* loginWithArgs(User* users, const char* username, const char* password) {
    User* currentUser = users;
    while (currentUser != NULL) {
        if (strcmp(currentUser->username, username) == 0 &&strcmp(currentUser->password, password) == 0) {
            // Authentication successful
            printf("Login successful. Welcome, %s!\n", username);
            clearScreen();
            return currentUser; // Return the authenticated user
        }
        currentUser = currentUser->next;
    }

    printf("Login failed. Please try again.\n");
    clearScreen();
    return NULL; // Authentication failed
}

char* getNextToken(char** input) {
    char* start = *input;
    char* end;
    char* token;

    // Skip leading spaces
    while (*start && *start == ' ') start++;

    if (*start == '<') {  // Quoted token
        start++;  // Skip the opening quote
        end = strchr(start, '>');  // Find the closing quote
        if (end) {
            *end = '\0';  // Terminate the token
            *input = end + 2;  // Move past the closing quote and space
        } else {
            // Handle error: unmatched quote
            printf("Error: Unmatched quote.\n");
            return NULL;
        }
        token = start;
    } else {  // Unquoted token
        end = strchr(start, ' ');  // Find the next space
        if (end) {
            *end = '\0';  // Terminate the token
            *input = end + 1;  // Move past the space
        } else {
            *input = start + strlen(start);  // No more tokens
        }
        token = start;
    }

    return (*token == '\0') ? NULL : token;
}

void displayBoards(const User* user) {
    printf("Available Boards:\n");
    const Board* currentBoard = user->boards;
    int boardCount = 0;
    while (currentBoard != NULL) {
        printf("%d. %s\n", ++boardCount, currentBoard->name);
        currentBoard = currentBoard->next;
    }
    if (boardCount == 0) {
        printf("No boards available.\n");
    }
}

Board* selectBoard(User* user) {
    printf("Enter the number of the board to select, or 0 to go back: ");
    int choice = atoi(dynamicInput());
    int boardIndex = 1;
    Board* currentBoard = user->boards;
    while (currentBoard != NULL) {
        if (boardIndex == choice) {
            return currentBoard; // Return the selected board
        }
        boardIndex++;
        currentBoard = currentBoard->next;
    }
    clearScreen();
    return NULL; // No board selected or invalid choice
}

void createBoard(User* user) {
    printf("Enter the name of the new board: ");
    char* boardName = dynamicInput();
    if (boardName != NULL && boardName[0] != '\0') {
        // Create and prepend the new board
        Board* newBoard = malloc(sizeof(Board));
        newBoard->name = boardName;
        newBoard->lists = NULL;
        newBoard->next = user->boards;
        newBoard->id = generateUniqueId();
        user->boards = newBoard;
        printf("Board '%s' created successfully.\n", boardName);
    } else {
        printf("Board creation cancelled.\n");
        free(boardName); // Free the input if the user entered an empty name
    }
}

void deleteBoard(User* user) {
    displayBoards(user); // Show all boards
    printf("Enter the number of the board to delete, or 0 to cancel: ");
    int choice = atoi(dynamicInput());
    int boardIndex = 1;
    Board* currentBoard = user->boards;
    Board* prevBoard = NULL;
    while (currentBoard != NULL) {
        if (boardIndex == choice) {
            // Found the board to delete
            if (prevBoard == NULL) {
                user->boards = currentBoard->next; // Board is the head of the user
            } else {
                prevBoard->next = currentBoard->next; // Bypass the current board
            }
            // Delete all lists in the board
            while (currentBoard->lists != NULL) {
                List* listToDelete = currentBoard->lists;
                currentBoard->lists = listToDelete->next;
                // Delete all tasks in the list
                while (listToDelete->tasks != NULL) {
                    Task* taskToDelete = listToDelete->tasks;
                    listToDelete->tasks = taskToDelete->next;
                    free(taskToDelete->name);
                    free(taskToDelete->priority);
                    free(taskToDelete->date);
                    free(taskToDelete);
                }
                free(listToDelete->name);
                free(listToDelete);
            }
            free(currentBoard->name);
            free(currentBoard);
            printf("Board deleted successfully.\n");
            return;
        }
        prevBoard = currentBoard;
        currentBoard = currentBoard->next;
        boardIndex++;
    }
    if (choice != 0) {
        printf("Board not found.\n");
    }
}

// Helper function to compare two tasks by their deadline.
int taskDeadlineComparator(const void* a, const void* b) {
    const TaskInfo* infoA = (const TaskInfo*)a;
    const Task* taskA = infoA->task;

    const TaskInfo* infoB = (const TaskInfo*)b;
    const Task* taskB = infoB->task;

    return strcmp(taskA->date, taskB->date); // Ascending order
}


char* getCurrentDate() {
    time_t now = time(NULL);
    struct tm* now_tm = localtime(&now);
    char* currentDate = malloc(11); // "YYYY-MM-DD" + null terminator
    if (currentDate != NULL) {
        strftime(currentDate, 11, "%Y-%m-%d", now_tm);
    }
    return currentDate;
}

void showUpcomingTasks(User* user) {
    if (user == NULL || user->boards == NULL) {
        fprintf(stderr, "User or boards is NULL.\n");
        return;
    }

    char* currentDate = getCurrentDate();
    TaskInfo* upcomingTasks = NULL;
    size_t taskCount = 0;

    // Iterate over all boards
    for (Board* board = user->boards; board != NULL; board = board->next) {
        // Iterate over all lists in the current board
        if (board->lists == NULL) continue;
        for (List* list = board->lists; list != NULL; list = list->next) {
            // Iterate over all tasks in the current list
            if (list->tasks == NULL) continue;
            for (Task* task = list->tasks; task != NULL; task = task->next) {
                // Check if the task's date is later than the current date
                if (strcmp(task->date, currentDate) > 0) {
                    // Reallocate memory for the upcomingTasks array
                    upcomingTasks = realloc(upcomingTasks, (taskCount + 1) * sizeof(TaskInfo));
                    if (upcomingTasks == NULL) {
                        fprintf(stderr, "Failed to allocate memory for upcomingTasks.\n");
                        return;
                    }

                    // Add the current task to the upcomingTasks array
                    upcomingTasks[taskCount].task = malloc(sizeof(Task));
                    *(upcomingTasks[taskCount].task) = *task; // Copy the task
                    upcomingTasks[taskCount].boardName = strdup(board->name); // Copy the board name
                    upcomingTasks[taskCount].listName = strdup(list->name); // Copy the list name
                    taskCount++;
                }
            }
        }
    }

    // Sort the upcomingTasks array by task deadline
    qsort(upcomingTasks, taskCount, sizeof(TaskInfo), taskDeadlineComparator);

    // Print out the upcoming tasks
    for (size_t i = 0; i < taskCount && i < 3; i++) {
        printf("%d) Task: %s, Priority: %s, Deadline: %s, Board: %s, List: %s\n", i+1, upcomingTasks[i].task->name, upcomingTasks[i].task->priority, upcomingTasks[i].task->date, upcomingTasks[i].boardName, upcomingTasks[i].listName);
    }

    // Free the allocated memory
    for (size_t i = 0; i < taskCount; i++) {
        free(upcomingTasks[i].task);
        free(upcomingTasks[i].boardName);
        free(upcomingTasks[i].listName);
    }
    free(upcomingTasks);
    free(currentDate);
}

void boardsMenu(User* user) {
    int choice;
    do {
        showUpcomingTasks(user);
        printf("1. View Boards\n2. Create Board\n3. Delete Board\n4. Exit\nChoose an option: ");
        choice = atoi(dynamicInput());

        switch (choice) {
            case 1:
                clearScreen();
                displayBoards(user);
                Board* selectedBoard = selectBoard(user);
                if (selectedBoard) {
                    clearScreen();
                    listsMenu(user, selectedBoard);
                }
                break;
            case 2:
                clearScreen();
                createBoard(user);
                clearScreen();
                break;
            case 3:
                clearScreen();
                deleteBoard(user);
                clearScreen();
                break;
            case 4:
                printf("Exiting to main menu.\n");
                clearScreen();
                break;
            default:
                printf("Invalid option. Please try again.\n");
                clearScreen();
                break;
        }
    } while (choice != 4);
}

void displayLists(const Board* board) {
    printf("Available Lists on Board '%s':\n", board->name);
    const List* currentList = board->lists;
    int listCount = 0;
    while (currentList != NULL) {
        printf("%d. %s\n", ++listCount, currentList->name);
        currentList = currentList->next;
    }
    if (listCount == 0) {
        printf("No lists available.\n");
    }
}

List* selectList(Board* board) {
    printf("Enter the number of the list to select, or 0 to go back: ");
    int choice = atoi(dynamicInput());
    int listIndex = 1;
    List* currentList = board->lists;
    while (currentList != NULL) {
        if (listIndex == choice) {
            return currentList; // Return the selected list
        }
        listIndex++;
        currentList = currentList->next;
    }
    clearScreen();
    return NULL; // No list selected or invalid choice
}

void createList(Board* board) {
    printf("Enter the name of the new list: ");
    char* listName = dynamicInput();
    if (listName != NULL && listName[0] != '\0') {
        // Create and prepend the new list
        List* newList = malloc(sizeof(List));
        newList->name = listName;
        newList->tasks = NULL;
        newList->next = board->lists; 
        newList->id = generateUniqueId();
        board->lists = newList;
        printf("List '%s' created successfully.\n", listName);
    } else {
        printf("List creation cancelled.\n");
        free(listName); // Free the input if the user entered an empty name
    }
}

void deleteList(Board* board) {
    displayLists(board); // Show all lists
    printf("Enter the number of the list to delete, or 0 to cancel: ");
    int choice = atoi(dynamicInput());
    int listIndex = 1;
    List* currentList = board->lists;
    List* prevList = NULL;
    while (currentList != NULL) {
        if (listIndex == choice) {
            // Found the list to delete
            if (prevList == NULL) {
                board->lists = currentList->next; // List is the head of the board
            } else {
                prevList->next = currentList->next; // Bypass the current list
            }
            // Delete all tasks in the list
            while (currentList->tasks != NULL) {
                Task* taskToDelete = currentList->tasks;
                currentList->tasks = taskToDelete->next;
                free(taskToDelete->name);
                free(taskToDelete->priority);
                free(taskToDelete->date);
                free(taskToDelete);
            }
            free(currentList->name);
            free(currentList);
            printf("List deleted successfully.\n");
            return;
        }
        prevList = currentList;
        currentList = currentList->next;
        listIndex++;
    }
    if (choice != 0) {
        printf("List not found.\n");
    }
}

void listsMenu(User* user, Board* board) {
    int choice;
    do {
        printf("1. View Lists\n2. Create List\n3. Delete List\n4. Exit\nChoose an option: ");
        choice = atoi(dynamicInput());

        switch (choice) {
            case 1: {
                clearScreen();
                displayLists(board);
                List* selectedList = selectList(board);
                if (selectedList) {
                    clearScreen();
                    tasksMenu(user, board, selectedList);
                }
                break;
            }
            case 2:
                clearScreen();
                createList(board);
                clearScreen();
                break;
            case 3:
                clearScreen();
                deleteList(board);
                clearScreen();
                break;
            case 4:
                printf("Exiting to board menu.\n");
                clearScreen();
                break;
            default:
                printf("Invalid option. Please try again.\n");
                clearScreen();
                break;
        }
    } while (choice != 4);
}

int isValidDate(char* date) {
    int yyyy, mm, dd;
    if (sscanf(date, "%4d-%2d-%2d", &yyyy, &mm, &dd) != 3) {
        return 0; // Incorrect format
    }
    if (yyyy < 1000 || yyyy > 9999 || mm < 1 || mm > 12 || dd < 1 || dd > 31) {
        return 0; // Invalid date
    }
    if ((mm == 4 || mm == 6 || mm == 9 || mm == 11) && dd == 31) {
        return 0; // April, June, September and November have 30 days
    }
    if (mm == 2) {
        int leap = yyyy % 400 == 0 || (yyyy % 100 != 0 && yyyy % 4 == 0);
        if (dd > 29 || (dd == 29 && !leap)) {
            return 0; // February has 28 days (29 in a leap year)
        }
    }
    return 1; // Valid date
}

void displayTasks(const List* list) {
    printf("Tasks in List '%s':\n", list->name);
    const Task* currentTask = list->tasks;
    int taskCount = 0;
    while (currentTask != NULL) {
        printf("%d. %s - Priority: %s, Deadline: %s\n", ++taskCount, currentTask->name, currentTask->priority, currentTask->date);
        currentTask = currentTask->next;
    }
    if (taskCount == 0) {
        printf("No tasks available.\n");
    }
}

Task* selectTask(List* list) {
    displayTasks(list); // Show all tasks
    printf("Enter the number of the task to select, or 0 to go back: ");
    int choice = atoi(dynamicInput());
    int taskIndex = 1;
    Task* currentTask = list->tasks;
    while (currentTask != NULL) {
        if (taskIndex == choice) {
            return currentTask; // Return the selected task
        }
        taskIndex++;
        currentTask = currentTask->next;
    }
    clearScreen();
    return NULL; // No task selected or invalid choice
}

void addTask(List* list) {
    printf("Enter the name of the new task or 'exit' to return: ");
    char* taskName = dynamicInput();
    if (taskName == NULL || strcmp(taskName, "exit") == 0 || taskName[0] == '\0') {
        free(taskName);
        return;
    }

    printf("Enter the priority (low, medium, high) of the task or 'exit' to return: ");
    char* priority = dynamicInput();
    if (priority == NULL || strcmp(priority, "exit") == 0 || priority[0] == '\0') {
        free(taskName);
        free(priority);
        return;
    }

    char* deadline;
    do {
        printf("Enter the deadline (YYYY-MM-DD) of the task or 'exit' to return: ");
        deadline = dynamicInput();
        if (deadline == NULL || strcmp(deadline, "exit") == 0 || deadline[0] == '\0') {
            free(taskName);
            free(priority);
            free(deadline);
            return;
        }
    } while (!isValidDate(deadline));

    // Create and prepend the new task
    Task* newTask = malloc(sizeof(Task));
    newTask->name = taskName;
    newTask->priority = priority;
    newTask->date = deadline;
    newTask->next = list->tasks;
    newTask->id = generateUniqueId();
    list->tasks = newTask;

    printf("Task '%s' added successfully.\n", taskName);
}

void editTask(List* list) {
    Task* selectedTask = selectTask(list);
    if (selectedTask) {
        printf("Editing Task '%s'. Enter new values or 'exit' to keep current values:\n", selectedTask->name);

        printf("Enter the new name of the task or 'exit' to keep current: ");
        char* newName = dynamicInput();
        if (newName != NULL && strcmp(newName, "exit") != 0 && newName[0] != '\0') {
            free(selectedTask->name);
            selectedTask->name = newName;
            printf("Task name updated successfully.\n");
        } else {
            free(newName); // No change made, free the input
        }

        printf("Enter the new priority (low, medium, high) or 'exit' to keep current: ");
        char* newPriority = dynamicInput();
        if (newPriority != NULL && strcmp(newPriority, "exit") != 0 && newPriority[0] != '\0') {
            free(selectedTask->priority);
            selectedTask->priority = newPriority;
            printf("Task priority updated successfully.\n");
        } else {
            free(newPriority); // No change made, free the input
        }

        char* newDeadline;
        do {
            printf("Enter the new deadline (YYYY-MM-DD) or 'exit' to keep current: ");
            newDeadline = dynamicInput();
            if (newDeadline == NULL || strcmp(newDeadline, "exit") == 0 || newDeadline[0] == '\0' || isValidDate(newDeadline)) {
                break; // Exit the loop if user cancels or enters a valid date
            }
            printf("Invalid date format. Please try again.\n");
            free(newDeadline); // Free the input if the date is invalid and prompt again
        } while (1);

        if (newDeadline != NULL && strcmp(newDeadline, "exit") != 0 && newDeadline[0] != '\0') {
            free(selectedTask->date);
            selectedTask->date = newDeadline;
            printf("Task deadline updated successfully.\n");
        } else {
            free(newDeadline); // No change made, free the input
        }
    } else {
        printf("Task editing cancelled.\n");
    }
}

void deleteTask(List* list) {
    displayTasks(list); // Show all tasks
    printf("Enter the number of the task to delete, or 0 to cancel: ");
    int choice = atoi(dynamicInput());
    int taskIndex = 1;
    Task* currentTask = list->tasks;
    Task* prevTask = NULL;
    while (currentTask != NULL) {
        if (taskIndex == choice) {
            // Found the task to delete
            if (prevTask == NULL) {
                list->tasks = currentTask->next; // Task is the head of the list
            } else {
                prevTask->next = currentTask->next; // Bypass the current task
            }
            free(currentTask->name);
            free(currentTask->priority);
            free(currentTask->date);
            free(currentTask);
            printf("Task deleted successfully.\n");
            return;
        }
        prevTask = currentTask;
        currentTask = currentTask->next;
        taskIndex++;
    }
    if (choice != 0) {
        printf("Task not found.\n");
    }
}

void moveTask(Board* board, List* currentList) {
    Task* selectedTask = selectTask(currentList);
    if (selectedTask) {
        printf("Select the destination list number or '0' to cancel: ");
        displayLists(board);
        List* targetList = selectList(board);
        if (targetList && targetList != currentList) {
            // Remove the task from the current list
            if (currentList->tasks == selectedTask) {
                // The task to move is the first task in the current list
                currentList->tasks = selectedTask->next;
            } else {
                // Find the task before the selected task
                Task* prevTask = currentList->tasks;
                while (prevTask && prevTask->next != selectedTask) {
                    prevTask = prevTask->next;
                }
                if (prevTask) {
                    prevTask->next = selectedTask->next;
                }
            }
            // Add the task to the target list
            selectedTask->next = targetList->tasks;
            targetList->tasks = selectedTask;

            printf("Task '%s' moved to list '%s'.\n", selectedTask->name, targetList->name);
        } else {
            printf("Task move cancelled.\n");
        }
    }
}

// Comparison function for sorting tasks by priority
int compareTasksByPriority(const void* a, const void* b) {
    const Task* taskA = *(const Task**)a;
    const Task* taskB = *(const Task**)b;
    const char* priorities[] = { "low", "medium", "high" };

    int priorityA = 0, priorityB = 0;
    for (int i = 0; i < 3; i++) {
        if (strcmp(taskA->priority, priorities[i]) == 0) priorityA = i;
        if (strcmp(taskB->priority, priorities[i]) == 0) priorityB = i;
    }

    return priorityB - priorityA; // Descending order
}

// Comparison function for sorting tasks by date
int compareTasksByDate(const void* a, const void* b) {
    const Task* taskA = *(const Task**)a;
    const Task* taskB = *(const Task**)b;

    // Assuming date is in "YYYY-MM-DD" format, lexicographical comparison is sufficient
    return strcmp(taskA->date, taskB->date); // Ascending order
}

// Function to sort tasks in a list based on the given comparison function
void sortTasks(List* list, int (*compFunc)(const void*, const void*)) {
    // Count the number of tasks
    int taskCount = 0;
    for (Task* task = list->tasks; task != NULL; task = task->next) {
        taskCount++;
    }

    if (taskCount <= 1) {
        return; // No need to sort if there's 0 or 1 task
    }

    // Convert linked list to array
    Task** tasksArray = malloc(taskCount * sizeof(Task*));
    Task* current = list->tasks;
    for (int i = 0; i < taskCount; i++) {
        tasksArray[i] = current;
        current = current->next;
    }

    // Sort the array of tasks
    qsort(tasksArray, taskCount, sizeof(Task*), compFunc);

    // Rebuild the linked list from the sorted array
    list->tasks = tasksArray[0];
    for (int i = 0; i < taskCount - 1; i++) {
        tasksArray[i]->next = tasksArray[i + 1];
    }
    tasksArray[taskCount - 1]->next = NULL;

    // Free the array
    free(tasksArray);
}

void sortTasksMenu(List* list) {
    if (list == NULL || list->tasks == NULL) {
        printf("No tasks to sort.\n");
        return;
    }

    printf("Sort tasks by:\n1) Priority (High to Low)\n2) Date (Nearest to Furthest)\n3) Cancel\nChoose an option: ");
    int sortChoice = atoi(dynamicInput());

    switch (sortChoice) {
        case 1:
            sortTasks(list, compareTasksByPriority);
            printf("Tasks have been sorted by priority.\n");
            break;
        case 2:
            sortTasks(list, compareTasksByDate);
            printf("Tasks have been sorted by date.\n");
            break;
        case 3:
            printf("Sort cancelled.\n");
            break;
        default:
            printf("Invalid option. Sort cancelled.\n");
            break;
    }
}

void tasksMenu(User* user, Board* board, List* list)  {
    int choice;
    do {
        displayTasks(list); // Show all tasks at the top of the menu
        printf("1. Add Task\n2. Edit Task\n3. Delete Task\n4. Move Task\n5. Sort Tasks\n6. Exit\nChoose an option: ");
        choice = atoi(dynamicInput());

        switch (choice) {
            case 1:
                clearScreen();
                addTask(list);
                clearScreen();
                break;
            case 2:
                clearScreen();
                editTask(list);
                clearScreen();
                break;
            case 3:
                clearScreen();
                deleteTask(list);
                clearScreen();
                break;
            case 4:
                clearScreen();
                moveTask(board, list);
                clearScreen(); // 'user' should be passed to tasksMenu or retrieved from the list
                break;
            case 5:
                clearScreen();
                sortTasksMenu(list); // Call the sortTasksMenu function
                clearScreen();
                break;
            case 6:
                printf("Exiting to list menu.\n");
                clearScreen();
                break;
            default:
                printf("Invalid option. Please try again.\n");
                clearScreen();
                break;
        }
    } while (choice != 6);
}

void printLogo() {
    printf("################################################################################################### \n");
    printf("  _    _   ___________    _______       ________          ___         ______          _______       \n");
    printf(" | |  | | ||___   ___|| ||       ))   ||        ||       // \\\\       ||     ))      ||       ))     \n");
    printf(" | |  | |      | |      ||        ))  ||        ||      //   \\\\      ||      ))     ||        ))    \n");
    printf(" | |  | |      | |      ||_______))   ||        ||     //     \\\\     ||_____))      ||         ))   \n");
    printf(" | |  | |      | |      ||       ))   ||        ||    //_______\\\\    ||     \\\\      ||         ))   \n");
    printf(" | |__| |      | |      ||        ))  ||        ||   //         \\\\   ||      \\\\     ||        ))    \n");
    printf(" \\\\____//      |_|      ||_______))   ||________||  //           \\\\  ||       \\\\    ||_______))     \n");
    printf("\n################################################################################################### \n\n");
}