#include <rpc/rpc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../maekawa.h"

#include <sys/file.h>
#define   LOCK_SH   1    /* shared lock */
#define   LOCK_EX   2    /* exclusive lock */
#define   LOCK_NB   4    /* don't block when locking */
#define   LOCK_UN   8    /* unlock */


extern __thread int errno;

int Id;//same as the client
int cur_ts[5] = {0};
char friendHost[24];
char replicationLog[50] = "replicationLog";

void replicationLogger(partsend* rec)
{
	FILE* f = NULL;
	f = fopen("tmpfile","a");
	if(f == NULL)
	{
		printf("Replication logging failed.\n");
		return;
	}
	fprintf(f,"%s %d %d %d %d %d %d %d %d\n",rec->name,rec->bytes,rec->ts[0],rec->ts[1],rec->ts[2],rec->ts[3],rec->ts[4],rec->sid,rec->rid);
	printf("Replication logged.\n");
	fclose(f);
}

int replicate(const char* partner)
{
	CLIENT* clnt;
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
	partsend rec;
	while(fscanf(f,"%s %d %d %d %d %d %d %d %d\n",
			rec.name,&(rec.bytes),&(rec.ts[0]),&(rec.ts[1]),&(rec.ts[2]),&(rec.ts[3]),&(rec.ts[4]),&(rec.sid),&(rec.rid))!=EOF)
	{
		int d = rec.sid;
		rec.sid = Id;
		if((clnt = clnt_create(friendHost, FTPROG, FTVER, "tcp"))==NULL)
		{
			replicationLogger(&rec);
			printf("Returned from replication.\n");
		}
		else
		{
			int* r = send_file_1(&rec, clnt);
			if(r == NULL || *r != 0)
			{
				replicationLogger(&rec);
			}
			else
			{
				printf("Replication Successful.\n");
			}
			clnt_destroy(clnt);
		}
		rec.sid = d;
	}
	fclose(f);
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
	char partner[20] = "partner";
	char accessor[20] = "accessor";
	int result;
	getId();
	FILE *file = NULL;
	file = fopen(partner,"r");
	if(file == NULL)
	{
		printf("Couldn't find the partner configuration file.\n");
		exit(1);
	}
	fscanf(file,"%s",friendHost);
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
	result = replicate(partner);
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
