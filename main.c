#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/types.h>   
#include <fcntl.h>
#include <readline/readline.h>
#include <sys/stat.h>

#define READLINE_BUFFER 1024
#define TOKEN_BUFFER 128
#define DELIMITER " \t\r\n\a" 
#define LIMIT 100		

int shell_help(char **args);
int shell_exit(char **args);
int shell_history(char **args);

//CommandTableStack contains history of commands typed in during the execution of the shell.
//LIMIT determines the total number of commands that could be saved.
char *CommandTableStack[LIMIT];
int stack_count = 0;

//Pushes command onto the stack after a successfull execution.
void push(char *CommandTableStack[],char **args){
	if(stack_count < LIMIT){
		CommandTableStack[stack_count] = malloc(TOKEN_BUFFER*sizeof(char *));
		strcpy(CommandTableStack[stack_count],args[0]);
		stack_count++;
		return ;
	}
	else{
		int i;
		for(i=1;i<stack_count;i++)
			strcpy(CommandTableStack[i-1],CommandTableStack[i]);
		strcpy(CommandTableStack[stack_count-1],args[0]);
		return ;
	}
}

//Inbuilt helper commands.
char *helper_commands[] ={
  "help",
  "history",
  "exit"
};

int(*helper_commands_pointers[]) (char **) ={
  &shell_help,
  &shell_history,
  &shell_exit
};

int helper_size = sizeof(helper_commands)/sizeof(char *);

//Shows some inbuilt commands.
int shell_help(char **args){
	int i;
	printf("Internally defined commands.\n");
	for(i=0;i<helper_size;i++)
		printf("%d : %s\n",i+1,helper_commands[i]);
	printf("Type help to see Inbuilt command list.\n");
	return true;
}

//Shows the history of commands typed in during the course of execution of shell.
int shell_history(char **args){
	int i;
	for(i=0;i<stack_count;i++){
		printf("%s\n",CommandTableStack[i]);
	}
	return true;
}

int shell_exit(char **args){
	return false;
}


//Executes a program/process and wait for it to terminate.
//return : returns 1 or true for the shell to continue execution.
int execute(char **args){
	pid_t pid;
	int status;
	/*
	fork()  creates  a new process by duplicating the calling process.  The
    new process, referred to as the child, is an  exact  duplicate  of  the
    calling  process.

    On success, the PID of the child process is returned in the parent, and
    0  is returned in the child.  On failure, -1 is returned in the parent,
    no child process is created, and errno is set appropriately.

    Refer to fork manual for further details.
    */
	pid = fork();

	/*
	perror prints a description for a system error code or for a storage
    engine (table handler) error code.

	*/

	/*
	waitpid()
	Wait for state changes in a child of the calling process , and obtain 
	information about the child whose state has changed. A state could be 
	changed by : 1. process termination 2. Termination by a signal 3. Any 
	other changes resulted because of signal.

	Refer to waitpid manual for further details.
	*/


	if(pid == 0){
	    if (execvp(args[0], args) == -1) {
	      perror("ERROR");
	    }
	    exit(EXIT_FAILURE);
	} 
	else if(pid < 0){
	    perror("Couldn't create a child process");
	} 
	else{
		do{
	    	int w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
	    	if(w == -1){
	    		perror("ERROR");
	    		exit(EXIT_FAILURE);
	    	}
	    }while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return true;
}

//Changes the current working directory.
int shell_cd(char **args){
	if (args[1] == NULL) {
    	fprintf(stderr, "specify a directory to move to\n");
  	} 
  	else{
    	if(chdir(args[1]) != 0){
      	perror("ERROR");
    	}
  	}	
  	return true;
}

//Reads a line from console/standard input.
char* shell_readline(){
	int buffsize = READLINE_BUFFER;
	int ctr = 0;
	char *buffer = malloc(buffsize*sizeof(char));
	char c;
	if(!buffer){
		fprintf(stderr,"shell allocation error\n");
		exit(EXIT_FAILURE);
	}

	while(1){
		c = getchar();
		if(c == EOF || c == '\n'){
			buffer[ctr] = '\0';
			return buffer;
		}
		else{
			buffer[ctr++] = c;
		}

		if(ctr >= READLINE_BUFFER){
			buffsize = buffsize + READLINE_BUFFER;
			buffer = realloc(buffer,buffsize);
			if(!buffer){
				fprintf(stderr,"shell allocation error\n");
				exit(EXIT_FAILURE);			
			}
		}
	}
}

//Breaks the input command.
char **shell_parseline(char *line){
	int ctr = 0;
	int buffsize = TOKEN_BUFFER;
	char **tokens = malloc(buffsize*sizeof(char *)),**tmp;
	char *token;
	if(!tokens){
		fprintf(stderr,"shell allocation error\n");
		exit(EXIT_FAILURE);		
	}
	token = strtok(line,DELIMITER);
	while(token != NULL){
	    tokens[ctr] = token;
	    ctr++;
	    if(ctr >= buffsize){
	    	buffsize += TOKEN_BUFFER;
	      	tmp = tokens;
	      	tokens = realloc(tokens,buffsize*sizeof(char*));
	      	if(!tokens) {
				free(tmp);
	        	fprintf(stderr,"shell allocation error\n");
	        	exit(EXIT_FAILURE);
	      	}
	    }
	    token = strtok(NULL,DELIMITER);
	}
	tokens[ctr] = NULL;
	return tokens;
}

int shell_execute(char **args){
	int i;
	if(args[0] == NULL)
		return true;
	for(i=0;i<helper_size;i++){
		if(strcmp(args[0],helper_commands[i])==0){
			return (*helper_commands_pointers[i])(args);
		}
	}
	//special care for command "cd".
	if(strcmp(args[0],"cd")==0){
		return shell_cd(args);
	}
	else
		return execute(args);
}

void main_shell(void){
	char *line;
	char **args;
	int status;
	do{
		printf(">>");
		line = shell_readline();
	    args = shell_parseline(line);
	    status = shell_execute(args);
	    if(args[0] != NULL)
	    	push(CommandTableStack,args);
    	free(line);
    	free(args);
	}while(status);
	return ;
}

int main(int argc, char const *argv[]){
	main_shell();
	return 0;
}