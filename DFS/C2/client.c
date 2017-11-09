#include <rpc/rpc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../maekawa.h"

#include <sys/file.h>
#define   LOCK_SH   1	/* shared lock */
#define   LOCK_EX   2	/* exclusive lock */
#define   LOCK_NB   4	/* don't block when locking */
#define   LOCK_UN   8	/* unlock */


extern __thread int errno;

int Id;
int cur_ts[5] = {0};
//Explicit Replication due to lack of Active Directory Service in NFS.
int idMap[4] = {0,2,3,1};
int revidMap[4] = {0,3,1,2};
char hostMap[4][24];
char replicationLog[50] = "replication_log.txt";
char accessor[20] = "accessor";

void sendfile(char *host, char *name, int serverno);
int get_file(char *host, char *name, int serverno);
int put_file(char *host, char *name, int serverno);
int list_files(const char* host,int serverno);

/* Clocking Utility */
char timestampFile[20] = "timestamp";
void getClock()
{
	if(access(timestampFile,F_OK ) == -1 ) //if doesn't exist, create new one.
	{
		FILE* f = fopen(timestampFile,"w");
		fprintf(f,"0 0 0 0 0");
		int i;
		for(i=0;i<5;i++)
			cur_ts[i]=0;
		fclose(f);
		return;
	}
	else
	{
		FILE* f = fopen(timestampFile,"r");
		fscanf(f,"%d %d %d %d %d",&cur_ts[0],&cur_ts[1],&cur_ts[2],&cur_ts[3],&cur_ts[4]);
		fclose(f);
	}
}


void setClock()
{
	FILE* f = fopen(timestampFile,"w");
	fprintf(f,"%d %d %d %d %d",cur_ts[0],cur_ts[1],cur_ts[2],cur_ts[3],cur_ts[4]);
	fclose(f);
}
/* End Clocking Utility */


/* Flat File Database Utility */

char fileDB[50] = "client_file_DB.csv";
int fileDBlen = 0;

typedef struct
{
	char filename[MAXLEN];
	int s1;
	int s2;
	int s3;
	int c1;
	int c2;
}record;

record fileDataBase[100]; // support for 100 files only

void tokenize(char* line, int* a)
{
	
	char *token = strtok(line, ",");
	if (token != NULL)
	{
		strcpy(fileDataBase[*a].filename,token);
		token = strtok(NULL, ",");
		if(token != NULL)
		{
			fileDataBase[*a].s1 = atoi(token);
			token = strtok(NULL, ",");
			if(token != NULL)
			{
				fileDataBase[*a].s2 = atoi(token);
				token = strtok(NULL, ",");
				if(token != NULL)
				{
					fileDataBase[*a].s3 = atoi(token);
					token = strtok(NULL, ",");
					if(token != NULL)
					{
						fileDataBase[*a].c1 = atoi(token);
						token = strtok(NULL, ",");
						if(token != NULL)
						{
							fileDataBase[*a].c2 = atoi(token);
						}
					}
				}
			}
		}
		
	}
	(*a)++;
	return;
}

void update(const char* name, const int* timestamp)
{
	FILE *stream = NULL;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
 
	if(access(fileDB, F_OK ) == -1 ) //if doesn't exist, create new one.
	{
		stream = fopen(fileDB,"w");
		fclose(stream);
	}

	stream = NULL;	
	
	while(stream == NULL)
		stream = fopen(fileDB, "r");
 	int i=0;
	while ((read = getline(&line, &len, stream)) != -1) {
		tokenize(line,&i);
	}
	fileDBlen = i;
	free(line);
	fclose(stream);
	if(strcmp(name,"")!=0)
	{
		FILE* output = NULL;
		output = fopen(fileDB, "w");
		if(output == NULL)
		{
			printf("Couldn't open the DB file.\n");
			exit(1);
		}
		int found = 0;
		for(i=0;i<fileDBlen;i++)
		{
			if(strcmp(name,fileDataBase[i].filename)==0)
			{
				fileDataBase[i].s1 = timestamp[0];
				fileDataBase[i].s2 = timestamp[1];
				fileDataBase[i].s3 = timestamp[2];
				fileDataBase[i].c1 = timestamp[3];
				fileDataBase[i].c2 = timestamp[4];
				found = 1;
			}
			fprintf(output,"%s,",fileDataBase[i].filename);
			fprintf(output,"%d,",fileDataBase[i].s1);
			fprintf(output,"%d,",fileDataBase[i].s2);
			fprintf(output,"%d,",fileDataBase[i].s3);
			fprintf(output,"%d,",fileDataBase[i].c1);
			fprintf(output,"%d\n",fileDataBase[i].c2);
		}
		if(found == 0) // new entry in the file database.
		{
			if(fileDBlen==100)
			{
				printf("MAXIMUM FILE RECORDS EXCEEDED.\n");
				exit(0);//down the server.
			}
			strcpy(fileDataBase[i].filename,name);
			fileDataBase[i].s1 = timestamp[0];
			fileDataBase[i].s2 = timestamp[1];
			fileDataBase[i].s3 = timestamp[2];
			fileDataBase[i].c1 = timestamp[3];
			fileDataBase[i].c2 = timestamp[4];
			fileDBlen++;
			fprintf(output,"%s,",fileDataBase[i].filename);
			fprintf(output,"%d,",fileDataBase[i].s1);
			fprintf(output,"%d,",fileDataBase[i].s2);
			fprintf(output,"%d,",fileDataBase[i].s3);
			fprintf(output,"%d,",fileDataBase[i].c1);
			fprintf(output,"%d\n",fileDataBase[i].c2);	
		}
		fclose(output);		
	}
}

/* End of flat file database utility */

/*Logging utility*/

char fileLog[50] = "client_file_log.txt";
void logger(const char* action,int sid, int rid, const char* name)
{
	int i;
	//log the transaction
	FILE *log = NULL;
	log = fopen(fileLog, "a");
	if(log == NULL)
	{
		printf("Couldn't log the event.\n");
		exit(1);
	}
	fprintf(log,"%s",action);
	for(i=0;i<5;i++)
		fprintf(log,"%d,",cur_ts[i]);
	fprintf(log,"%d,",sid);
	fprintf(log,"%d,",rid);
	fprintf(log,"%s\n",name);
	fclose(log);
}

/*End of logging utility*/

/* Replication utility */

void replicator(const char* filepath, const int serverno)
{
	FILE* f = NULL;
	f = fopen(replicationLog,"a");
	if(f == NULL)
	{
		printf("Couldn't open the replication log.\n");
		return;
	}
	//race condition avoided by accessor 
	fprintf(f,"%s ",filepath);
	int k;
	for(k=0;k<5;k++)
		fprintf(f,"%d ",cur_ts[k]);
	fprintf(f,"%d ",Id);//sender Id
	fprintf(f,"%d ",serverno);//origin server Id
	fprintf(f,"%d\n",idMap[serverno]);//receiver server Id
	fclose(f);
}

/* End of Replication utility */

/* File copy utility */
void filecopy(const char* dest, const char* src)
{
	char s[3*MAXLEN] = "storage/";
	char d[3*MAXLEN] = "storage/";
	strcat(s,src);
	strcat(d,dest);
	// Open one file for reading
	FILE* fptr1 = fopen(s, "r");
	// Open another file for writing
	FILE* fptr2 = fopen(d, "w");
	// Read contents from file
	char c = fgetc(fptr1);
	while (c != EOF)
	{
		fputc(c, fptr2);
		c = fgetc(fptr1);
	}
	fclose(fptr1);
	fclose(fptr2);
}
/* End of file copy utility */

int get_file(char *host, char *name, int serverno) //just to ensure all servers are not operating on same domain, we use serverno.
{
	vtimestamp ts;
	int stalecopy = 0;
	CLIENT *clnt;
	int total_bytes = 0, write_bytes;
	readfile_res *result;
	request req;
	FILE *file;
	strcpy(req.name,name);
	req.start = 0;
	int i;
	req.sid = Id;
	req.rid = serverno; 
	clnt = clnt_create(host, FTPROG, FTVER, "tcp");
	if (clnt == NULL) 
	{
		 clnt_pcreateerror(host);
		 exit(1);
	}
	printf("Connected to the server.\n");
	file = fopen("storage/tmpinput", "wb");
	if(file == NULL)
	{
		printf("Couldn't find the file.\n");
		return -1;
	}
	for(i=0;i<5;i++)
		req.ts[i] = cur_ts[i];
	while (!stalecopy) 
	{
		req.start = total_bytes;
		result = retrieve_file_1(&req, clnt);		
		if (result == NULL) 
		{
			clnt_perror(clnt, host);
			return -2;
		}
		if (result->errno != 0) 
		{
			errno = result->errno;
			perror(name);
			return -3;
		}
		write_bytes = fwrite(result->readfile_res_u.part.data, 1, result->readfile_res_u.part.bytes, file);
		total_bytes += result->readfile_res_u.part.bytes;
		for(i=0;((i<5)&&(!stalecopy));i++)
		{
			if(ts[i] <= result->readfile_res_u.part.ts[i])
				ts[i] = result->readfile_res_u.part.ts[i];
			else
				stalecopy = 1;				
		}
		if (result->readfile_res_u.part.bytes < MAXLEN) 
			break;
	}
	fclose(file);
	if(stalecopy)
	{
		printf("Stalecopy found at server %d.\n",serverno);
		cur_ts[Id-1]++;
		setClock();
		sendfile(host,name,serverno);
	}
	else
	{
		for(i=0;i<5;i++)
			cur_ts[i] = ts[i];
		setClock();
		filecopy(name,"tmpinput");
		remove("storage/tmpinput");
		printf("Copied file Successfully.\n");
	}
	logger("G,",req.sid,req.rid,req.name);
	update(name,cur_ts);
	clnt_destroy(clnt);
	return 0;
}

// It is assumed that the client will responsibly perform list file to check if his own file is outdated, get the new copy
// and then performs put file. Server is not concurrent and hence is not robust.
int put_file(char *host, char *name, int serverno)//just to ensure all servers are not operating on same domain, we use serverno.
{
	CLIENT *clnt;
	char data[1024];
	int total_bytes = 0, read_bytes;
	int *result;
	partsend part;
	FILE *file;
	int i;
	part.sid = Id;
	part.rid = serverno;
	clnt = clnt_create(host, FTPROG, FTVER, "tcp");
	if (clnt == NULL) 
	{
		printf("Couldn't connect to server.\n");
		clnt_pcreateerror(host);
		return -2;
	}
	char fnama[3*MAXLEN]="storage/";
	strcat(fnama,name);
	file = fopen(fnama, "rb");
	strcpy(part.name,name);
	for(i=0;i<5;i++)
		part.ts[i] = cur_ts[i];
	while (1) 
	{
		part.bytes = total_bytes;
		read_bytes = fread(part.data, 1, MAXLEN, file);
		total_bytes += read_bytes;
		part.bytes = read_bytes;
		result = send_file_1(&part, clnt);
		if (result == NULL) 
		{
			clnt_perror(clnt, host);
			return -3;
		}
		if (*result != 0) 
		{
			errno = *result;
			perror(name);
			return -4;
		}
		for(i=0;i<5;i++)
			cur_ts[i] = part.ts[i];//critical
		setClock();
		if (read_bytes < MAXLEN) 
			break;
	}
	fclose(file);
	logger("P,",part.sid,part.rid,part.name);
	update(name,cur_ts);
	clnt_destroy(clnt);
	return 0;
}

void sendfile(char *host, char *name, int serverno)
{
	int updateResult=0, replicationResult = 0;
	updateResult = put_file(host,name,serverno);
	if(updateResult == 0)
	{
		//explicit Replication : automated if server is up and running
		replicationResult = put_file(hostMap[serverno],name,idMap[serverno]);
		printf("Replication result : %d\n",replicationResult);
	}
	else
	{
		printf("Server %d is down.\n",serverno);
		replicator(name,revidMap[serverno]);
	}
	if(replicationResult != 0)
	{
		printf("Replication failed.\n");
		replicator(name,serverno);
		
	}
}

int list_files(const char* host,int serverno)
{
	CLIENT *clnt;
	listfile_res *result;
	request req;
	strcpy(req.name,"LIST");
	req.start = 0;
	req.sid = Id;
	req.rid = serverno; 
	clnt = clnt_create(host, FTPROG, FTVER, "tcp");
	if (clnt == NULL) 
	{
		 printf("\nUnable to connect to the server.\n");
		 clnt_pcreateerror(host);
		 return -1;
	}
	else
	{
		printf("Connected to the server %d. \n",serverno);
	}
	result = listfile_1(&req, clnt);		
	if (result == NULL) 
	{
		printf("\nUnable to list the files from the server.\n");
		clnt_perror(clnt, host);
		return -2;
	}
	else
	{
		printf("Got the results from the server %d. \n",serverno);
	}
	int i;
	if (result->errno != 0) 
	{
		errno = result->errno;
		printf("Some error in displaying the results from the server %d. \n",serverno);
		perror("LIST");
		return -3;

	}
	printf("Files from server %d : \n",serverno);
	for(i=0;i<result->listfile_res_u.fileListing.len;i++)
	{
		printf("%s\t",result->listfile_res_u.fileListing.list[i]);
		int j;
		for(j=0;j<4;j++)
			printf("%d,",result->listfile_res_u.fileListing.ts[i][j]);
		printf("%d\n",result->listfile_res_u.fileListing.ts[i][j]);
	}
	printf("End of files from Server %d\n",serverno);
	clnt_destroy(clnt);
	return 0;
}

int read_command()
{
	char host[MAXLEN],command[MAXLEN], filepath[MAXLEN];
	int serverno;
	printf("\n> ");
	fflush(stdin);
	scanf("%s",command);
	if(strcmp(command,"list") == 0)
	{
		FILE* f = NULL;
		if(access(accessor, F_OK) == -1 ) 
		{
			printf("Couldn't find the access lock file.\n");
			exit(1);
		}
		f = fopen(accessor,"r");
		if(f == NULL)
		{
			printf("Unable to read from access file.\n");
			exit(1);
		}
		int getlock = -1;
		while(getlock == -1)
		{
			printf("Trying to get a lock on the access file. Probably maintenance by cron is on.\n");
			getlock = flock(fileno(f),LOCK_EX);
		}
		printf("Locked the access file.\n");
		//critical section
		int s1,s2,s3;
		s1 = list_files(hostMap[3],1);
		s2 = list_files(hostMap[1],2);
		s3 = list_files(hostMap[2],3);
		//end critical section
		int release = -1;
		while(release == -1)
		{
			printf("Trying to release the lock acquired on the access file. Probably cron maintenance ended.\n");
			release = flock(fileno(f), LOCK_UN);
		}
		printf("Released the access file.\n");
		fclose(f);
		return 0;
	}
	if(strcmp(command, "exit") == 0)
	{
		return 2;
	}
	scanf(" %s %s %d",host,filepath,&serverno);
	if(serverno<=0 || serverno>=4)
	{
		printf("Choose a server number in [1,2,3].");
	}
	else
	{
		int retval = 0;
		FILE* f = NULL;
		if(access(accessor, F_OK) == -1 ) 
		{
			printf("Couldn't find the access lock file.\n");
			exit(1);
		}
		f = fopen(accessor,"r");
		if(f == NULL)
		{
			printf("Unable to read from access file.\n");
			exit(1);
		}
		int getlock = -1;
		while(getlock == -1)
		{
			printf("Trying to get a lock on the access file. Probably maintenance by cron is on.\n");
			getlock = flock(fileno(f),LOCK_EX);
		}
		printf("Locked the access file.\n");
		if (strcmp(command, "get") == 0) 
		{
			retval = get_file(host,filepath,serverno);
		} 
		else if(strcmp(command, "put") == 0)
		{
			cur_ts[Id-1]++;
			setClock();
			sendfile(host,filepath,serverno);
			retval = 0;
		}
		else
		{
			printf("The available commands are : \nlist\nexit\nget <servername> <filename> <servernum>\nput <servername> <filename> <servernum>\n");
			retval = -1;
		}
		//end critical section
		int release = -1;
		while(release == -1)
		{
			printf("Trying to release the lock acquired on the access file. Probably cron maintenance ended.\n");
			release = flock(fileno(f), LOCK_UN);
		}
		printf("Released the access file.\n");
		fclose(f);
		return retval; 
	}
}

void getId()
{
	FILE* f = NULL;
	f = fopen("Id","r");
	if(f == NULL)
	{
		printf("Id file for the client is not set.\n");
		exit(0);
	}
	if(fscanf(f,"%d",&Id)==EOF)
	{
		printf("Id file is present but Id is not set.\n");
		exit(0);
	}
	fclose(f);
}

int main(int argc, char *argv[])
{
	getId();
	getClock();
	update("",cur_ts);
	char config[20] = "config";
	int result;
	FILE *file = NULL;
	file = fopen(config,"r");
	if(file == NULL)
	{
		printf("Couldn't find the configuration file.\n");
		exit(1);
	}
	strcpy(hostMap[0],"");
	fscanf(file,"%s %s %s",hostMap[3],hostMap[1],hostMap[2]);
	fclose(file);
	while(TRUE) 
	{
		result = read_command();
		switch(result)
		{
			case 2:
				exit(0);
				break;
			default:
				continue;
		}
	}
   	return 0;
}
