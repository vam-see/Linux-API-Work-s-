/* 
# Author Vamsee
# Title : File Chat Program
# Syracuse University 
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

char *inFile, *outFile;
struct stat st;
int errnum;
char* in_line = NULL;
size_t line_capacity = 0;
time_t rawtime;
struct tm *timeinfo;
size_t fbytes;
ssize_t bytes_written, bytes_read, last_read;
char *cTime;
char *outString;
char buffer[BUFSIZ];
int inFiledesc, outFiledesc;
volatile sig_atomic_t switchMode,chkAlarm;

void handler(int sig)
{
	if (switchMode)
	{
		switchMode = 0;
	}
	else
	{
		switchMode = 1;
	}
	chkAlarm = 1;
}

void fdHandler(int sig)
{
	//Close file descriptors
	close(inFiledesc);
	close(outFiledesc);
	_exit(0);
}

int main(int argc, char *argv[])
{
	switchMode = 1;
	if ( argv[optind] == NULL ){
		printf("%s <input file> <output file> \n", argv[0]);
		_exit(0);
	}

	inFile = argv[1];
	outFile = argv[2];

	if ( stat(inFile, &st) < 0 )
	{
			perror("Input file error");
			_exit(0);	
	}

	siginterrupt(SIGALRM, 1);
	if (signal(SIGALRM, handler) == SIG_ERR)
	{
		perror("Signal error with alarm");
	}

	if (signal(SIGUSR1, handler) == SIG_ERR)
	{
		perror("Signal error with alarm");
	}

	if (signal(SIGINT, fdHandler) == SIG_ERR)
	{
		perror("Signal error with SIGUSR1");
	}

	chkAlarm = 1;
	last_read = 0;
	alarm(3);

	//Open file descriptors
	inFiledesc = open(inFile, O_RDONLY);
	outFiledesc = open(outFile, O_APPEND | O_RDWR | O_CREAT, S_IRWXU);

	while(1){

		if (chkAlarm)
		{
			chkAlarm = 0;
			alarm(3);
		}
		if (switchMode)
		{
			//Read from console and write to output file
			if (getline(&in_line, &line_capacity, stdin) != -1)
			{
			time ( &rawtime );
  			timeinfo = localtime ( &rawtime );
  			cTime = asctime (timeinfo);
  			cTime[strlen(cTime) -1 ] = 0;
  			outString = strcat(cTime, ", ");
  			outString = strcat(outString, in_line);
  			outString = strcat(outString, "\n");
			fbytes = strlen(outString);
 			bytes_written = write(outFiledesc, outString, fbytes);
 				if (bytes_written != fbytes)
 				{
 					perror("Write error");
 				}
			}
			else
			{
				clearerr(stdin);
			}	
		}
		else
		{
			//Read from input file and write to output file
			bytes_read = read(inFiledesc, &buffer, BUFSIZ);
			if (bytes_read <= 0)
			{
				raise(SIGUSR1);
			}
			if(bytes_read != last_read)
			{
				printf("%s", buffer);
				memset(buffer, 0, sizeof(buffer));
				last_read = bytes_read;	
			}
		}				 		
	}
	return 0;
}