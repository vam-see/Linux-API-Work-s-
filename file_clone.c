/* Author Vamsee
# SUID 749435970
# Title : File Clone
# Advanced System Programming */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>

int fflag = 0;
struct stat st;
int infiledesc;
int outfiledesc;
mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
int srcdir = 0;
int destdir = 0;
int infileLen;
size_t fbytes;
ssize_t bytes_towrite;
ssize_t bytes_written;
char buffer[4096];

int main( int argc, char *argv[] ){

	DIR *d;
	struct dirent *dir;
	int c;

	if( argc != 4){
		printf("./projectone.out <source> <destination> -f \n");
		exit(0);
	}

	//Check if source exists or cannot be read
	char *infile = argv[1];
	infiledesc = open(infile, O_RDONLY, mode);
	if ( infiledesc < 0){
		perror("Error opening Source");
		exit(0);
	}

	//Check if destination is file or directory
	char *outfile =  argv[2];
	if ( lstat(outfile, &st) == 0)
	{
		switch	(st.st_mode & S_IFMT){
			case S_IFDIR :	d = opendir(argv[2]);
							if ( d != NULL)
							{
								char *path = malloc( strlen(outfile) + strlen(infile) + 10);
								strcpy(path, outfile);
								strcat(path, infile);
								outfiledesc = open(path, O_WRONLY | O_CREAT, mode);
								if (outfiledesc < 0)
								{
									perror("Unable create file");
								}
								//free(path);
							}
							else
							{
								perror("Error opening Destination");
							}
							
			break;
			case S_IFREG : remove(outfile);
						   outfiledesc = open(outfile, O_WRONLY | O_CREAT, mode);
						   if ( outfiledesc < 0){
								perror("Error opening Destination");
								exit(0);
							}
			break;
			default : printf("Unknown\n"); 
		}
	}
	else{
		perror("Error opening Destination");
		exit(0);
	}
	
	
	while (( c = getopt (argc, argv, ":f")) != -1 ){
		switch(c){
			case 'f' : fflag = 1;
			break;

			case '?' : printf("Invalid option(s)\n");
					   printf("./projectone.out <source> <destination> -f\n");
			break;
			
			default : abort();
		}
	}

	if(fflag == 1){
	struct utimbuf ut;
	
	uid_t uid;
	gid_t gid;
	if( stat(infile, &st) == 0){
		struct passwd *pwd;
		struct group *grp;
		pwd = getpwuid(st.st_uid);
		grp = getgrgid(st.st_gid);
		ut.actime = st.st_atime;
		ut.modtime = st.st_mtime;
		mode = st.st_mode;
		fbytes = st.st_size;
		chmod(outfile, mode);
		uid = pwd->pw_uid;
		gid = grp->gr_gid;
		if(chown(outfile, uid, gid) == -1){
			perror("CHOWN");
		}
		

		while( bytes_towrite = read(infiledesc, buffer, sizeof buffer), bytes_towrite > 0){
			if(write(outfiledesc, buffer, bytes_towrite) != bytes_towrite)	{
				perror("Error writing to Destination");
			}
		}
		utime(outfile, &ut);
				
	}
	printf("File Cloning Successful\n");	
	}
	return 0;
}