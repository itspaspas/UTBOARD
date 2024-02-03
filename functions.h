#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <io.h>
#include <errno.h>
#include <time.h>

typedef struct Task {
    long id;
    char* name;
    char* priority;
    char* date;
    int modified;
    struct Task* next;
} Task;

typedef struct List {
    long id;
    char* name;
    int modified;
    struct List* next;
    Task* tasks;
} List;

typedef struct Board {
    long id;
    char* name;
    int modified;
    struct Board* next;
    List* lists;
} Board;

typedef struct User {
    char* username;
    char* password;
    int modified;
    struct User* next;
    Board* boards;
} User;


// ... (other includes and definitions)

// Function prototypes (add these)
int isValidDate(char* date);
long generateUniqueId();
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
void showUpcomingTasks(const User* user);
int compareTasksByPriority(const void* a, const void* b);
int compareTasksByDate(const void* a, const void* b);
void sortTasks(List* list, int (*compFunc)(const void*, const void*));
void sortTasksMenu(List* list);
void printLogo();