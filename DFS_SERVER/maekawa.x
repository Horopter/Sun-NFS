const MAXLEN = 1024;

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

typedef opaque filepart[MAXLEN];

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

union readfile_res switch (int errno) {
    case 0:
        partreceive part;
    default:
        void;
};

struct fileListing {
	filename list[MAXLEN];
	vtimestamp ts[MAXLEN];
	int len;
};

typedef struct fileListing fileListing;

union listfile_res switch (int errno) {
    case 0:
        fileListing fileListing;
    default:
        void;
};

program FTPROG {
    version FTVER {
        readfile_res retrieve_file(request *) = 1;
        int send_file(partsend *) = 2;
	listfile_res listfile(request *) = 3;
    } = 1;
} = 0x31240000;
