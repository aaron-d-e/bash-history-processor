/**
* file: bhp.c
* author: Aaron Evans
* course: CSI 3336
* assignment: project 5
* due date: 10/11/2026
*
* notes: had way too much fun with this assignment - might have gone overkill. would like
* to have seen some more in-depth instructions since it was fairly broad (unless that was
* the point). wasn't sure how far to take it which is why i spent hours researching and deb-
* ugging the system command.
*/

/*
 * read and parse 'string by string'
 * check if its a valid command - which and builtins
 * check if its a new word
 * if so, make new CmdRec
 * if not, increase count
 * reset on pipe and newline
 *
 */

/* not going to have a shrink function in my vector since we arent deleting CmdRec ever */

#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <regex.h>
#include <sys/types.h>

typedef int bool;
#define false 0
#define true 1

/* struct given in project requirements */
typedef struct {
	/* name of the command  - can hold 12 characters + null terminator */
	char cmdName[ 13 ];
	int cmdFound; /* num of specified command defined above */
} CmdRec;

/* vector that holds CmdRec array as well as size and capacity for resize purposes */
typedef struct {
	CmdRec **records;
	size_t size;
	size_t capacity;
} CmdVector;

/**
* initCmdVector() 
*
* return a valid pointer to a new CmdVector instance
*
* Parameters:
*
* return: pointer to the newly created CmdVector
*/
CmdVector* initCmdVector(){
	CmdVector* container = malloc(sizeof(CmdVector));
	if(container == NULL){
		const char* str = "initCmdVector: container not initialized";
		printf("%s\n", str);
		return NULL;
	}
	container->size = 0;
	container->capacity = 5;
	container->records = malloc(sizeof(CmdRec*) * container->capacity);
	
	return container;
}

/**
* resizeCmdVector() 
*
* doubles the capacity of a CmdVector 
*
* Parameters: existing CmdVector instane
*
* return: pointer to the same container with updated capacity (could be void)
*/
CmdVector* resizeCmdVector(CmdVector* container){
	int i;
	int newCapacity = container->capacity * 2;
	CmdRec** newRecords = malloc(sizeof(*newRecords) * newCapacity);
	if(newRecords == NULL){
		const char* str = "new records array not created properly";
		printf("%s\n", str);
		return NULL;
	}
		

	for(i = 0; i < container->size; i++){
		newRecords[i] = container->records[i]; 
	}

	free(container->records);
	container->records = newRecords;
	container->capacity = newCapacity;


	return container;
}


/**
* resizeCmdVector() 
*
* vector push_back command for a CmdVector container 
*
* Parameters: existing CmdVector instance, existing CmdRec instance
*
* return: void - updated in place
*/
void push_back(CmdVector* container, CmdRec* record){
	if (container->size == container->capacity){
		container = resizeCmdVector(container);	
		if(container == NULL){
			const char* str = "container could not be resized";
			printf("%s\n", str);
			exit(-1);
		}
	}
	container->records[container->size] = record;
	container->size++;
}


/**
* strEqual()
*
* custom strcmp 
*
* Parameters: two const char* to be compared  
*
* return: bool - whether they are equal or not
*/
bool strEqual(const char* s0, const char* s1){
	int i = 0;
	while(s0[i] != '\0' || s1[i] != '\0'){
		if(s0[i] != s1[i]) return false;
		i++;
	}

	return true;
}

/**
* findCmdIndex()
*
* find the index of an existing command, or =-1 for no match
*
* Parameters: existing CmdVector, and command to be added to the container as a CmdRec
*
* return: int - either -1 for no match, or index where the command can be found
*/
int findCmdIndex(CmdVector* container, const char* command){
	int i;
	for(i = 0; i < container->size; i++){
		if (strEqual(container->records[i]->cmdName, command) == true){
			return i; /* match found */
		}
	}
	return -1;
}


/**
* isValidCommand()
*
* checks whether the command in bash_history is a real command
* 
* Parameters: const char * - command to check validity of
*
* return: bool - whether the command is valid or not
*/
bool isValidCommand(const char* command){
	bool path = true;
	bool builtin = true;

	int bash;
	/* man 3 setenv */
	setenv("CMD", command, 1);
	/* man 3 system */
	bash = system( "which -- \"$CMD\" > /dev/null 2>&1");
	
	if(bash == -1 || !WIFEXITED(bash)) {
		path = false;
	} /* the command itself failed to run */

	if(WEXITSTATUS(bash) != 0) {
		path = false;
	} /* the command could not be found on path */

	/* check builtin */
	bash = system( "bash -c 'compgen -b | grep -- \"$CMD\"' > /dev/null 2>&1");

	if(bash == -1 || !WIFEXITED(bash)) {
		builtin = false;
	}

	if(WEXITSTATUS(bash) != 0) {
		builtin = false;
	} 
	
	unsetenv("CMD");

	if(!builtin && !path) return false;

	return true;
}

/**
* customStrCpy()
*
* custom strcpy
* 
* Parameters: dest - where the copy goes, src - 'string to be copied'
*
* return: void - copied in place
*/
void customStrCpy(char* dest, const char* src){
	int i = 0;
	while(src[i] != '\0'){
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}

/**
* customStrCpy()
*
* insert a valid command into the container's record array 
*
* Parameters: container - existing CmdVector, const char* - command to be inserted 
*
* return: void - inserted in place
*/
void insertCommand(CmdVector* container, const char* command){
	int index;

	bool isCommand = isValidCommand(command);
	if(!isCommand) return; /* return early if the command is invalid */

	index = findCmdIndex(container, command);

	if(index != -1){
		/* index here holds the matching CmdRec in container->records */
		container->records[index]->cmdFound++;
	} else {
		CmdRec* record = malloc(sizeof(CmdRec));
		customStrCpy(record->cmdName, command);
		record->cmdFound = 1;
		push_back(container, record);
	}
}

/**
* printCmdVector()
*
* print the contents of the vector specified by the project requriements
* 
* Parameters: container - existing CmdVector
*
* return: void - printed to stdout
*/
void printCmdVector(CmdVector* container){
	int i;
	for(i = 0; i < container->size; i++){
		printf("%-12s %4d\n", container->records[i]->cmdName, container->records[i]->cmdFound);
	}
}


int main() {
	int ch;
	int i;
	char* buffer = malloc(sizeof(char) * 13);
	int bufIndex = 0;
	bool maybeCommand = true; /* used to check for possible commands - resets after a newline or a pipe */

	CmdVector* container = initCmdVector();
	if (container == NULL){
		const char* str = "container not initialized";
		printf("%s\n", str);
		return 1;
	}
	if (container->records == NULL) {
		const char* str = "container's container is not initialized";
		printf("%s\n", str);
		return 1;
	}

	while((ch = fgetc(stdin)) != EOF){
		if( ch == '\n' || ch == ' ' || ch == '|' ){
			if(bufIndex > 0){
				if(maybeCommand && bufIndex <= 12){
					buffer[bufIndex] = '\0';
					insertCommand(container, buffer);
				}
				maybeCommand = false;
			}
			bufIndex = 0;
			if(ch == '\n' || ch == '|'){
				maybeCommand = true; /* restarts the check */
			}
		} else {
			if (bufIndex <= 12) buffer[bufIndex] = (char)ch;
			bufIndex++;
		}


	}

	if(bufIndex > 0 && bufIndex <= 12){
		buffer[bufIndex] = '\0';
		insertCommand(container, buffer);
	}

	printCmdVector(container);

	/* frees */
	for(i = 0; i < container->size; i++){
		free(container->records[i]);
	}

	free(container->records);
	free(container);
	free(buffer);


	return 0;
}
