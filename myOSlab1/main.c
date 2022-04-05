 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <sys/types.h>


#define delimiters " \n\t\r\a"
#define slice_buffer_size 100

int count;
char *input;
char **ptrINPUT; // for the input command but sliced


void loop(void);
char *enter_input();
void input_cd(char **input);
char **seperateString(char* input);
void execute(char **ptrINPUT);
void handler(int sig);

void handler( int sig) // #include <signal.h> will do the remaining
{
            	FILE *fptr;
            	fptr = fopen("logFILE.text","w+");
            	if(fptr == NULL)
               	{
                  	printf("Error!");
                  	exit(0);
               	}
            	fprintf(fptr,"process %s is terminated, this is the log file \n ", input);
            	//fclose(fptr);


}


int main()
{
	loop();
	return 0;
}

void loop(void)
{

	int processSTATUS=1;
	do
	{
    	input = enter_input();
    	ptrINPUT = seperateString(input);
    	execute(ptrINPUT); //// execute the input command
	}while(processSTATUS); // keep the shell working until the user type exit or press close

}




//getting string input from the user as a command:
char* enter_input(){
//signed size buffer to get command from the user when the actual size is unknown using function read
	ssize_t   readBytes;
	char buffer[BUFSIZ]; //BUFSIZ is defined already in stdio.h, we don't need to assign anything to it.
	char *input;

	write(1,">> ",4); // funny outputs if u change this values especially the 4
	readBytes = read(0,buffer,BUFSIZ - 1); //  0 if we are not reading from a file.
	buffer[readBytes - 1] = '\0'; //null to use (strsep / strtok) to slice it.
	input = buffer;
	return input;
}

void input_cd(char **input)
{
	if (input[1] == NULL)
	{
    	fprintf(stderr, "We Expect argument after \"cd\"\n");
    	exit(EXIT_FAILURE);
	}
	else
	{
    	if (chdir(input[1]) != 0)   //It returns 0 if success
    	{
        	perror("Error");
        	exit(EXIT_FAILURE);
    	}
	}
}

// to separated the command input to be ready for executing.
char **seperateString(char* input)
{

	count= 0;
	char *ptr = NULL; //a string pointer to loop the input
	char **storedINPUT = malloc(BUFSIZ * sizeof(char*)); //to store word by word


// loop the null terminated input in enter_input()
	do{

            	ptr = strsep(&input ,delimiters); // strsep points to the next letter after the delimiter so it slices up the command.
            	storedINPUT[count++] = ptr; // accumulating the string word by word

   	}while(ptr);

	return storedINPUT;
}

void execute(char **ptrINPUT){
	pid_t pid; // process id.
	int processSTATUS;


	pid = waitpid(-1, &processSTATUS, WNOHANG);   	// (-1) means wait for any child process, WNOHANG means that waitpid should return immediately.. instead of waiting if no child process is noticed..
	if (pid > 0) // 1== parent process
	{
    	printf(" (waitpid) kills child process id num: %d\n", pid);
	}
// if the 1st word of the command:
//cd:
	if(strcmp(ptrINPUT[0],"cd") == 0) //0 if indentical
	{
    	input_cd(ptrINPUT);
	}
// exit:
	else if(strcmp(ptrINPUT[0],"exit") == 0) // identical = 1 if strcmp == 0.
	{
    	exit(EXIT_SUCCESS);
	}
	else
	{
// Checking if the command has & at the end. before the null by 1.  count points to the last element at (size - 1).
                    	int and_operator_background = (  strcmp(ptrINPUT[count-2],"&") == 0); // identical = 1 if strcmp == 0; // to flag if the child process is working in the background or foreground.
                    	if(and_operator_background)
                    	{
                                        	ptrINPUT[count-2] = NULL; // put a Null before executing the code.
                    	}

                    	if(strlen(ptrINPUT[0])>0) //True is there is already a command and the user didn't hit enter by mistake.
                    	{
                        	pid = fork();
                        	if(pid == -1)
                        	{
                                        	fprintf(stderr,"Error while creating a child process.\n");
                                        	exit(EXIT_FAILURE);
                        	}
                        	else if (pid == 0) //child process.
                        	{
                                        	if(execvp(ptrINPUT[0], ptrINPUT) == -1) //Checking if the user entered a wrong command.
                                        	{
                                                	perror("Execution Error. Wrong Code.");
                                        	}
                                        	exit(EXIT_SUCCESS);
                        	}
                        	else if (!and_operator_background) //True if there was not any  & ampersand in the code, the child process will be working at the foreground.
                        	{
                                        	printf("The Process ID of the parent process is %d\n",getppid());
                                        	signal(SIGCHLD,handler);
                                        	// we can use 0 in waitpid as:
                                        	pid = waitpid(pid, &processSTATUS, 0); //as The parent process will wait for the child process to be terminated


                                        	if (pid > 0) //if waitpid() was successful. The value returned indicates the process ID of the child process whose status information was recorded in the storage pointed to by stat_loc.


                                        	{
                                            	printf("waitpid reaped child pid %d\n", pid); //The Child in the foreground is terminated and you can use shell now.
                                        	}
                        	}
    	}
	}
}



