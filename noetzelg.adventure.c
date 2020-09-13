/*********************************************************************
** Author: Greg Noetzel
** Date: 8/26/2020
** Description: adventure game to step through room files created by
                 noetzel.buildrooms.c
*********************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_CONNECTIONS     6
#define MIN_CONNECTIONS     3
#define NUM_ROOMS           7

struct Room{
    char name[12];
    char type[16];
    char connections[MAX_CONNECTIONS+1][16];
    int numConnections; 
};

//globals
char directoryName[256];
struct Room graph[NUM_ROOMS];

//prototypes
void printRoom(struct Room);

//determines the most recent modified directoy prefaced with "noetzelg.rooms"
//format mostly adapted from lecture
void getFolder(){
  struct stat dirStat;
  char tempDirName[256];

  // Open the directory
  DIR* currDir = opendir(getcwd(directoryName, sizeof(directoryName)));
  struct dirent *aDir;
  time_t lastModifTime;
  int i = 0;

  /* The data returned by readdir() may be overwritten by subsequent calls to  readdir()  
  for  the  same  directory stream. So we will copy the name of the directory to the variable directoryName
  */ 
  while((aDir = readdir(currDir)) != NULL){
    // Use strncmp to check if the directory name matches the prefix
    if(strncmp("noetzelg.rooms.", aDir->d_name, strlen("noetzelg.rooms.")) == 0){
      stat(aDir->d_name, &dirStat);
      // Check to see if this is the directory with the latest modification time
      if(i == 0 || difftime(dirStat.st_mtime, lastModifTime) > 0){
        lastModifTime = dirStat.st_mtime;
        memset(tempDirName, '\0', sizeof(tempDirName));
        strcpy(tempDirName, aDir->d_name);
      }
      i++;
    }
  }

  closedir(currDir);
  
  strcpy(directoryName, tempDirName);
}

//splits line string into two parts at the : char. label string and val string also stripped of spaces and carriage returns
void splitLine(char *line, char* label, char* val){
    int i;
    char * pch;

    pch = strtok(line, ":");
    strcpy(label, pch);
    pch = strtok (NULL, " \n");
    strcpy(val, pch);
}

//itterates thru file names in direcroty identified by getFolder() and assign rooms structs those names
void assnRoomNames(){
    DIR *dir;
    struct dirent *ent;
    int n = 0;

    if ((dir = opendir (directoryName)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if(strlen(ent->d_name) > 2){
                strcpy(graph[n].name,ent->d_name);
                n++;
            }
        }
    }
}

//parses file contents in directory identified by getFolder and assigns contents to Room 
//structs with corresponding names assgned by assnRoomNames()
//implementation 
void importData(){
    FILE *roomFile;
    int i;
    int nCon;
    char lineBuffer[32];
    char keyBuffer[16];
    char valBuffer[16];

    getFolder();

    assnRoomNames();

    chdir(directoryName);


    for(i = 0;i < NUM_ROOMS; i++){
        roomFile = fopen(graph[i].name,"r");

        //checks if file was opened 
        if(roomFile == NULL){
            printf("%s file not found\n", graph[i].name);
            return;
        }

        memset(lineBuffer,'\0',sizeof(lineBuffer));
        nCon = 0;

        //itterate thru each line in file
        while(fgets(lineBuffer,sizeof(lineBuffer), roomFile) != NULL){
            //clear buffers
            memset(keyBuffer,'\0',sizeof(keyBuffer));
            memset(valBuffer,'\0',sizeof(valBuffer));

            splitLine(lineBuffer, keyBuffer, valBuffer);

            //if line is a connection
            if (strcmp(keyBuffer,"CONNECTION 0") >= 1 && strcmp(keyBuffer,"CONNECTION 0") <= 7){ // fill in connections.
                
                strcpy(graph[i].connections[nCon], valBuffer);
                nCon++;
            }
            //if line is a room type
            else if(strcmp(keyBuffer,"ROOM TYPE") == 0){
                strcpy(graph[i].type, valBuffer);
            }
        }

        //assign number of connections tallied above
        graph[i].numConnections = nCon;

        fclose(roomFile);
    }
}

//iterates thru graph for rooms matching passed room type, returns address of first found
struct Room* getRoomByType(char* type){
    int i;
    for (i = 0; i < NUM_ROOMS; i++){
        if (strcmp(type, graph[i].type) <=  2){
            return &graph[i];
        }
    }
    return NULL;
}

//iterates thru graph for rooms matching passed room name, returns address of first found
struct Room* getRoomByName(char* name){
    int i;
    for (i = 0; i < NUM_ROOMS; i++){
        if (strcmp(name, graph[i].name) == 0){
            return &graph[i];
        }
    }
    return NULL;
}

//Begins game
void runGame(){
    int steps = 0; //step counter
    char pathString[256]; //tracks the path taken
    memset(pathString,'\0',sizeof(pathString));
    struct Room * curRoom; //current room pointer
    int win = 0; //game win flag

    //set current room to start room
    char typebuff[16] = "START_ROOM";
    curRoom = getRoomByType(typebuff);

    //start loop
    while (win == 0){
        int i;
        char inBuffer[16];
        int validIn = 0;

        printf("CURRENT LOCATION: %s\n", curRoom->name);

        printf("POSSIBLE CONNECTIONS: ");
        for (i = 0; i < curRoom->numConnections; i++){
            printf("%s, ", curRoom->connections[i]);
        }

        printf("WHERE TO? >");
        memset(inBuffer,'\0',sizeof(inBuffer));

        while (validIn == 0){
            scanf("%s", inBuffer);
            printf("\n\n");
            for (i=0; i < curRoom->numConnections; i++){
                if (strcmp(inBuffer, curRoom->connections[i]) == 0){    //if input string is found
                    curRoom = getRoomByName(curRoom->connections[i]);   //current room becomes input room
                    strcat(pathString, curRoom->name);                  //add room to pathString
                    strcat(pathString, "\n");
                    steps++;                                            //increpent stap count
                    validIn = 1;                                        //raist valid input flag
                }
            }
            if(validIn == 0){   //if input not valid
                    printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n >");
                }
        }
        
        if (strcmp(curRoom->type, "END_ROOM") == 0){  //check if ending has been reached
            printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
            printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
            printf("%s", pathString);
            win = 1; //rais win flag
        }
    }
}

int main() {
    //rand seed
    srand(time(NULL));
    
    importData();

    runGame();

    printRooms();

    return 0;
}