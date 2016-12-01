//proj3.c
//author jslink

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAXRATS 5
#define MAXROOMS 8

struct vbentry {
int iRat; /* rat identifier */
int tEntry; /* time of entry into room */
int tDep; /* time of departure from room */
};

//rats prototype
void Rats(void *arg);

//global variables
time_t global_start_time = 0;
pthread_t rat_ids[MAXRATS]; //array of "rat" thread ids
sem_t room_ids[MAXROOMS]; //array of "room" semaphore ids
sem_t guestbook_sems[MAXROOMS];
struct vbentry RoomVB[MAXROOMS][MAXRATS]; //array of room visitors books
int delay[MAXROOMS];
int room_count = 0;
char mazetype;
int total_time = 0;
int guestcount[MAXROOMS];

//semaphore for TryToEnterRoom
sem_t trySem;

int main(int argc, char *argv[]){
	
	int numrats = atoi(argv[1]);
	if((*argv[2] == 'i')||(*argv[2] == 'd')){
		mazetype = *argv[2];
	}
	else{
		printf("Please specify a number of rats and an algorithm type.\n");
		printf("The algorithm type can be either i or d.\n");
		exit(1);
	}
	global_start_time = time(NULL);
	//exit if there's too many rats
	if(numrats > MAXRATS){
		perror("Too many rats");
		exit(1);
	}
	
	int i=0;
	
	//open rooms file, error if un-openable
	FILE *rooms_file;
	rooms_file = fopen("rooms", "r");
	if(rooms_file == NULL){
		perror("Can't open file 'rooms'");
		exit(1);
	}
	
	/*Read in room specifications from file
	and create semaphores accordingly. */
	while(!feof(rooms_file)){
		int cap = 0;
		int d = 0;
		if(fscanf(rooms_file, "%d %d", &cap, &d) != 2){
			break;
		}
		//printf("%d %d\n", cap, d);
		delay[room_count] = d;
		if(sem_init(&room_ids[room_count], 0, cap) <0){
			perror("sem_init");
			exit(1);
		}
		guestcount[room_count] = 0;
		room_count++;
		//printf("%d\n", room_count);
		if(room_count == MAXROOMS){
			break;
		}
	}
	
	fclose(rooms_file);
	
	//make semaphores to protect guestbooks
	for(i=0;i<room_count;i++){
		if(sem_init(&guestbook_sems[i], 0, 1) <0){
			perror("sem_init");
			exit(1);
		}
	}
	
	//make semaphore for TryToEnterRoom
	if(sem_init(&trySem, 0, 1) <0){
			perror("sem_init");
			exit(1);
	}
	
	//make threads
	for(i=0;i<numrats;i++){
		if(pthread_create(&(rat_ids[i]), NULL, (void *)&Rats, (void *)i) != 0){
			perror("pthread_create");
			exit(1);
		}
	}
	
	//rejoin threads
	for(i=0;i<numrats;i++){
		(void)pthread_join((rat_ids[i]),NULL);
		//print stuff
		
	}
	
	//destroy semaphores and print out guestbook info
	for(i=0;i<room_count;i++){
		int capacity;
		sem_getvalue(&room_ids[i], &capacity);
		printf("Room %d [%d %d]:", i, capacity, delay[i]);
		int j = 0;
		for(j=0;j<numrats;j++){
			printf(" %d %d %d;", RoomVB[i][j].iRat, RoomVB[i][j].tEntry, RoomVB[i][j].tDep);
		}
		printf("\n");
		(void)sem_destroy(&room_ids[i]);
	}
	
	//gather and print out remaining information
	int total_delay = 0;
	for(i=0;i<room_count;i++){
		total_delay = total_delay + delay[i];
	}
	total_delay = total_delay * numrats;
	printf("Total traversal time: %d seconds, compared to ideal time: %d seconds.\n", total_time, total_delay);
	
	//destroy remaining semaphores
	for(i=0;i<room_count;i++){
		(void)sem_destroy(&guestbook_sems[i]);
	}
	(void)sem_destroy(&trySem);
	
	return 0;
}

/* Leave room method 
	records data in vbentry array and frees up
	a spot in the room. */
void LeaveRoom(int i_Rat, int iRoom, int tEnter){
	sem_wait(&guestbook_sems[iRoom]);
	
	RoomVB[iRoom][guestcount[iRoom]].iRat =  i_Rat;
	RoomVB[iRoom][guestcount[iRoom]].tEntry = tEnter;
	RoomVB[iRoom][guestcount[iRoom]].tDep = (int)(time(NULL) - global_start_time);
	guestcount[iRoom]++;
	sem_post(&guestbook_sems[iRoom]);
	sem_post(&room_ids[iRoom]);
}

/* Enter room method 
	allows a rat thread into a room if there is 
	an open spot, then waits the appropriate time
	before leaving the room. */
void EnterRoom(int iRat, int iRoom){
	sem_wait(&room_ids[iRoom]);
	time_t t_enter = time(NULL) - global_start_time;
	sleep(delay[iRoom]);
	LeaveRoom(iRat, iRoom, (int) t_enter);
}

/* Rats method
	this is called to send the rats into their 
	rooms */
void Rats(void *arg){
	
	int j = 0; 
	int numrats = (int)arg;
	if(mazetype == 'i'){ //enter in-order
		for(j=0;j<room_count;j++){
			//printf("Rat entering in-order.\n");
			EnterRoom(numrats, j);
		}
	}
	else if(mazetype == 'd'){ //enter distributed
		for(j=0;j<room_count;j++){
			if(numrats+j>=room_count){
				//printf("Rat entering distributed.\n");
				EnterRoom(numrats, numrats+j-room_count);
			}
			else{
				//printf("Rat entering distributed.\n");
				EnterRoom(numrats, numrats+j);
			}
		}
	}
	
	int completion_time = (int)time(NULL) - global_start_time;
	printf("Rat %d finished the maze in %d seconds.\n", numrats, completion_time);
	total_time = total_time + completion_time;
	
}

//Try to enter room method
int TryToEnterRoom(int iRat, int iRoom){
	sem_wait(&trySem);
	int space = 0;
	sem_getvalue(&room_ids[iRoom] , &space);
	if(space < 1){
		sem_post(&trySem);
		return -1;
	}
	else{
		//able to enter room
		sem_post(&trySem);
		EnterRoom(iRat, iRoom);
		return 0;
	}
}
