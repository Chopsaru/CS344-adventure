/*********************************************************************
** Author: Greg Noetzel
** Date: 8/26/2020
** Description: program to build room files for noetzelg.adventure.c
*********************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

//globals
#define MAX_CONNECTIONS     6
#define MIN_CONNECTIONS     3
#define NUM_ROOMS           7

//all posible room names couresy of Patrick Rothfuss.
char* roomNames[10] = {
    "Mews",
    "Hollows",
    "Mains",
    "Archives",
    "Fishery",
    "Medica",
    "Haven",
    "Eolian",
    "Underthg",
    "Ankers"
};  

struct Room{
    char name[9];
    char type[10];
    struct Room* connections[MAX_CONNECTIONS];
    int numConnections; 
    int full;
};

struct Room graph[NUM_ROOMS];

//prototypes
int IsSameRoom(struct Room, struct Room);
struct Room*  GetRandomRoom();
int CanAddConnectionFrom(struct Room);
int ConnectionAlreadyExists(struct Room*, struct Room*);
void shuffle(char**, int);

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
int IsGraphFull(){
    int i;
    int count = 0;
    for (i = 0; i < NUM_ROOMS; i++){
        if (graph[i].full == 1)
        count++;
    }
    if (count >= NUM_ROOMS){
        return 1;
    }
    return 0;
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct Room* x, struct Room* y){
    x->connections[x->numConnections] = y;
    x->numConnections++;
    if (x->numConnections >= 3){
        x->full = 1;
    }
}

// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection(){
    struct Room *A;
    struct Room *B;

    while(1){
        A = GetRandomRoom();

        if (CanAddConnectionFrom(*A) == 1)
        break;
    }

    do{
        B = GetRandomRoom();
    }
    while(CanAddConnectionFrom(*B) == 0 || IsSameRoom(*A, *B) == 1 || ConnectionAlreadyExists(A, B) == 1);

    ConnectRoom(A, B);
    ConnectRoom(B, A); 
}

// Returns a random Room, does NOT validate if connection can be added
struct Room * GetRandomRoom(){
    return &graph[rand() % NUM_ROOMS];
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
int CanAddConnectionFrom(struct Room x){
    if (x.numConnections < 6){
        return 1;
    }
    return 0;
}
// Returns true if a connection from Room x to Room y already exists, false otherwise
int ConnectionAlreadyExists(struct Room* x, struct Room* y){
    int i;
    if (x->connections[0] == NULL){
            return 0;
    }
    for (i=0; i < x->numConnections; i++){
        if (x->connections[i] == y){
            return 1;
        }
    }
    return 0;
}

// Returns true if Rooms x and y are the same Room, false otherwise
int IsSameRoom(struct Room x, struct Room y){
    if (strcmp(x.name, y.name) == 0){
        return 1;
    }
    return 0;
}

//initializes all room structs in graph
void initGraph(){
    int i;
    int j;
    shuffle(roomNames, 10);

    for(i = 0; i < NUM_ROOMS; i++){
        graph[i].numConnections = 0;
        graph[i].full = 0;
        strcpy(graph[i].type, "MID_ROOM");
        strcpy(graph[i].name, roomNames[i]);

        for(j = 0; j < MAX_CONNECTIONS; j++){
            graph[i].connections[i] = NULL;
        }
    }
    strcpy(graph[0].type, "START_ROOM");
    strcpy(graph[NUM_ROOMS - 1].type, "END_ROOM");
}

//shuffles passed array
//from https://stackoverflow.com/questions/6127503/shuffle-array-in-c
void shuffle(char**array, int n){
    if (n > 1) 
    {
        int i;
        for (i = 0; i < n - 1; i++) 
        {
          int j = i + rand() / (RAND_MAX / (n - i) + 1);
          char* t = array[j];  
          array[j] = array[i];
          array[i] = t;
        }
    }
}

//fills all room connections via addrandomConnection();
void connectRooms(){
    while (IsGraphFull() == 0){
        AddRandomConnection();
    }
}

//generates room directory and prints all room structs to files labeled by roomname
void printRoomFiles(){
    int i;
    int j;
    FILE * roomFile;
    char dirName[24];
    
    sprintf(dirName, "noetzelg.rooms.%d",rand() % 100000);

    mkdir(dirName, 0777);
    chdir(dirName);

    for(i = 0; i < NUM_ROOMS; i++){
        roomFile = fopen(graph[i].name, "w");
        fprintf(roomFile,"ROOM NAME: %s\n", graph[i].name);
        for(j = 0; j < graph[i].numConnections; j++){
            fprintf(roomFile, "CONNECTION %d: %s\n", j+1, graph[i].connections[j]->name);
        }
        fprintf(roomFile,"ROOM TYPE: %s\n",graph[i].type);

        fclose(roomFile);
    }
}


int main() {
    //rand seed
    srand(time(NULL));
 
    initGraph();
    connectRooms();
    printRoomFiles();

    return 0;
}