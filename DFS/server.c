#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../maekawa.h"

#include <sys/file.h>
#define   LOCK_SH   1    /* shared lock */
#define   LOCK_EX   2    /* exclusive lock */
#define   LOCK_NB   4    /* don't block when locking */
#define   LOCK_UN   8    /* unlock */

extern __thread int errno;

int Id;
vtimestamp cur_ts = {0};

char timestampFile[20] = "timestamp";

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

/* Clocking Utility */

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

char fileDB[50] = "server_file_DB.csv";
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
			return;
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

char fileLog[50] = "server_file_log.txt";
//log the transaction
void logger(const char* action,int sid, int rid, const char* name,int seek)
{
	int i;
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
	fprintf(log,"%s,",name);
	fprintf(log,"%d\n",seek);
	fclose(log);
}

/*End of logging utility*/

readfile_res* retrieve_file_1_svc(request *req, struct svc_req *rqstp)
{
	getId();
	getClock();
	update("",cur_ts);
	if(req->rid == Id)
	{
		FILE *file;
		char data[1024];
		int bytes,i;
		static readfile_res res;
		char fn[3*MAXLEN] = "storage/";
		strcat(fn,req->name);
		file = fopen(fn, "rb");
		if (file == NULL) 
		{
			res.errno = errno;
			return (&res);
		}
		fseek (file, req->start, SEEK_SET);
		bytes = fread(res.readfile_res_u.part.data, 1, 1024, file);
		res.readfile_res_u.part.bytes = bytes;
		res.readfile_res_u.part.sid = Id;
		res.readfile_res_u.part.rid = req->sid;
		res.errno = 0;
		fclose(file);
		logger("S,",req->sid,req->rid,req->name,req->start);
		return (&res);	
	}
}

int* send_file_1_svc(partsend *rec, struct svc_req *rqstp)
{
	getId();
	getClock();
	update("",cur_ts);
	int i;
	if(rec->rid == Id)
	{
		FILE *file;
		int write_bytes;
		static int result;
		//update the timestamp
		for(i=0;i<5;i++)
			cur_ts[i] = rec->ts[i];
		setClock();
		char fn[3*MAXLEN] = "storage/";
		strcat(fn,rec->name);
		if(rec->isStart == 1)// replace the file
		{
			file = fopen(fn, "w");
			if (file == NULL) 
			{
				result = errno;
				return &result;
			}
			fclose(file);
			rec->isStart = 0;
		}
		file = fopen(fn, "a");
		if (file == NULL) 
		{
			result = errno;
			return &result;
		}
		write_bytes = fwrite(rec->data, 1, rec->bytes, file);
		fclose(file);
		result = 0;
		logger("R,",rec->sid,rec->rid,rec->name,rec->bytes);
		update(rec->name,cur_ts);
		return &result;
	}
}

listfile_res* listfile_1_svc(request *req, struct svc_req *rqstp)
{
	getId();
	getClock();
	update("",cur_ts);
	if(req->rid == Id)
	{
		static listfile_res fl;
		fl.listfile_res_u.fileListing.len = fileDBlen;
		int i;
		for(i=0;i<fileDBlen;i++)
		{
			strcpy(fl.listfile_res_u.fileListing.list[i],fileDataBase[i].filename);
			fl.listfile_res_u.fileListing.ts[i][0] = fileDataBase[i].s1;
			fl.listfile_res_u.fileListing.ts[i][1] = fileDataBase[i].s2;
			fl.listfile_res_u.fileListing.ts[i][2] = fileDataBase[i].s3;
			fl.listfile_res_u.fileListing.ts[i][3] = fileDataBase[i].c1;
			fl.listfile_res_u.fileListing.ts[i][4] = fileDataBase[i].c2;
		}
		return &fl;
	}
}
