/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _MAEKAWA_H_RPCGEN
#define _MAEKAWA_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif

#define MAXLEN 1024

typedef char filename[MAXLEN];

typedef int vtimestamp[5];

typedef int sender_id;

typedef int recv_id;

struct request {
	filename name;
	int start;
	vtimestamp ts;
	sender_id sid;
	recv_id rid;
};
typedef struct request request;


typedef char filepart[MAXLEN];

struct partreceive {
	filepart data;
	int bytes;
	vtimestamp ts;
	sender_id sid;
	recv_id rid;
};
typedef struct partreceive partreceive;


struct partsend {
	filename name;
	filepart data;
	int bytes;
	vtimestamp ts;
	sender_id sid;
	recv_id rid;
	int isStart;
	int source;
};
typedef struct partsend partsend;


struct readfile_res {
	int errno;
	union {
		partreceive part;
	} readfile_res_u;
};
typedef struct readfile_res readfile_res;

struct fileListing {
	filename list[MAXLEN];
	vtimestamp ts[MAXLEN];
	int len;
};
typedef struct fileListing fileListing;


struct listfile_res {
	int errno;
	union {
		fileListing fileListing;
	} listfile_res_u;
};
typedef struct listfile_res listfile_res;

#define FTPROG 0x31240000
#define FTVER 1

#if defined(__STDC__) || defined(__cplusplus)
#define retrieve_file 1
extern  readfile_res * retrieve_file_1(request *, CLIENT *);
extern  readfile_res * retrieve_file_1_svc(request *, struct svc_req *);
#define send_file 2
extern  int * send_file_1(partsend *, CLIENT *);
extern  int * send_file_1_svc(partsend *, struct svc_req *);
#define listfile 3
extern  listfile_res * listfile_1(request *, CLIENT *);
extern  listfile_res * listfile_1_svc(request *, struct svc_req *);
extern int ftprog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define retrieve_file 1
extern  readfile_res * retrieve_file_1();
extern  readfile_res * retrieve_file_1_svc();
#define send_file 2
extern  int * send_file_1();
extern  int * send_file_1_svc();
#define listfile 3
extern  listfile_res * listfile_1();
extern  listfile_res * listfile_1_svc();
extern int ftprog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_filename (XDR *, filename);
extern  bool_t xdr_vtimestamp (XDR *, vtimestamp);
extern  bool_t xdr_sender_id (XDR *, sender_id*);
extern  bool_t xdr_recv_id (XDR *, recv_id*);
extern  bool_t xdr_request (XDR *, request*);
extern  bool_t xdr_request (XDR *, request*);
extern  bool_t xdr_filepart (XDR *, filepart);
extern  bool_t xdr_partreceive (XDR *, partreceive*);
extern  bool_t xdr_partreceive (XDR *, partreceive*);
extern  bool_t xdr_partsend (XDR *, partsend*);
extern  bool_t xdr_partsend (XDR *, partsend*);
extern  bool_t xdr_readfile_res (XDR *, readfile_res*);
extern  bool_t xdr_fileListing (XDR *, fileListing*);
extern  bool_t xdr_fileListing (XDR *, fileListing*);
extern  bool_t xdr_listfile_res (XDR *, listfile_res*);

#else /* K&R C */
extern bool_t xdr_filename ();
extern bool_t xdr_vtimestamp ();
extern bool_t xdr_sender_id ();
extern bool_t xdr_recv_id ();
extern bool_t xdr_request ();
extern bool_t xdr_request ();
extern bool_t xdr_filepart ();
extern bool_t xdr_partreceive ();
extern bool_t xdr_partreceive ();
extern bool_t xdr_partsend ();
extern bool_t xdr_partsend ();
extern bool_t xdr_readfile_res ();
extern bool_t xdr_fileListing ();
extern bool_t xdr_fileListing ();
extern bool_t xdr_listfile_res ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_MAEKAWA_H_RPCGEN */
