#include <rpc/rpc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "maekawa.h"

#include <sys/file.h>
#define   LOCK_SH   1    /* shared lock */
#define   LOCK_EX   2    /* exclusive lock */
#define   LOCK_NB   4    /* don't block when locking */
#define   LOCK_UN   8    /* unlock */


extern __thread int errno;

int Id;//same as the client
int cur_ts[5] = {0};
//Explicit Replication due to lack of Active Directory Service in NFS.
int idMap[4] = {0,2,3,1};
char hostMap[4][24];
char replicationLog[50] = "replication_log.txt";

/*Logging utility*/

char fileLog[50] = "client_file_log.txt";
void logger(const char* action,int sid, int rid, const char* name)
{
	int i;
	//log the transaction
	FILE *log = NULL;
	while(log == NULL) //critical section entry
	{
		printf("Waiting..\n");
		log = fopen(fileLog, "a");
	}
	int lock = -1;
	while(lock == -1)
	{
		printf("Acquiring a lock on the log file.\n");
		lock = flock(fileno(log), LOCK_EX);
	}
	//critical section
	fprintf(log,"%s",action);
	for(i=0;i<5;i++)
		fprintf(log,"%d,",cur_ts[i]);
	fprintf(log,"%d,",sid);
	fprintf(log,"%d,",rid);
	fprintf(log,"%s\n",name);
	//end critical section
	int release = -1;
	while(release == -1)
	{
		printf("Trying to release the lock acquired on the log file. \n");
		release = flock(fileno(log), LOCK_UN);
	}
	//promptly exit critical section
	fclose(log);
}

/*End of logging utility*/

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
			cur_ts[i] = part.ts[i];
		if (read_bytes < MAXLEN) 
			break;
	}
	fclose(file);
	logger("P,",part.sid,part.rid,part.name);
	clnt_destroy(clnt);
	return 0;
}

int checkPartner(const char* host,int serverno)
{
	CLIENT *clnt;
	clnt = clnt_create(host, FTPROG, FTVER, "tcp");
	if (clnt == NULL) 
	{
		 printf("\nUnable to connect to the server.\n");
		 clnt_pcreateerror(host);
		 return -1;
	}
	clnt_destroy(clnt);
	return 0;
}

int replicate()
{
	int sid,rid,cid,retval=0;
	char filename[MAXLEN];
	FILE* f = NULL;
	FILE* g = NULL;
	if(access(replicationLog, F_OK) == -1 ) 
	{
		printf("Couldn't find the replication log file.\n");
		return -2;
	}
	f = fopen(replicationLog,"r");
	if(f == NULL)
		return -1;
	g = fopen("tmpfile","w");
	if(g == NULL)
		return -1;
	while(fscanf(f,"%s %d %d %d %d %d %d %d %d\n",filename,&cur_ts[0],&cur_ts[1],&cur_ts[2],&cur_ts[3],&cur_ts[4],&cid,&sid,&rid)!=EOF)
	{
		if(cid == Id && rid == idMap[sid])
		{
			int key = checkPartner(hostMap[sid],idMap[sid]);
			if(key == -1)
			{
				printf("Server number %d with IP address %s is not up.\n",idMap[sid],hostMap[sid]);
				fprintf(g,"%s %d %d %d %d %d %d %d %d\n",filename,cur_ts[0],cur_ts[1],cur_ts[2],cur_ts[3],cur_ts[4],cid,sid,rid);
				retval = -3;
			}
			if(key == 0)
			{
				put_file(hostMap[sid],filename,idMap[sid]);
			}
		}
	}
	fclose(f);
	fclose(g);
	//Rewrite the pending logs. Race condition prevented by the accessor file.
	f = fopen("tmpfile","r");//hidden file actually
	if(f == NULL)
		return -1;
	g = fopen(replicationLog,"w");
	if(g == NULL)
		return -1;
	char c = fgetc(f);
	while (c != EOF)
	{
		fputc(c, g);
		c = fgetc(f);
	}
	remove("tmpfile");
	return retval;
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
	char config[20] = "config";
	char accessor[20] = "accessor";
	int result;
	getId();
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
		printf("Trying to get a lock on the access file.\n");
		getlock = flock(fileno(f),LOCK_EX);
	}
	printf("Locked the access file.\n");
	result = replicate();
	switch(result)
	{
		case -1:
			printf("Issue with replication files\n");
			break;
		case -2:
			printf("Nothing to replicate.\n");
			break;
		case -3:
			printf("One or more files were not replicated.");
			break;
		default:
			printf("Replication Successful.\n");

	}
	int release = -1;
	while(release == -1)
	{
		printf("Trying to release the lock acquired on the access file. \n");
		release = flock(fileno(f), LOCK_UN);
	}
	printf("Released the access file.\n");
	fclose(f);
   	return 0;
}
