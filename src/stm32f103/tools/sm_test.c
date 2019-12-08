#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <assert.h>
#include <getopt.h>
#include <Windows.h>

char *sm_ibuf_name = "putty_share_memory_in";
char *sm_obuf_name = "putty_share_memory_out";

#define SM_INPUT_BUF_SIZE	(4096)
#define SM_OUTPUT_BUF_SIZE	(4096)

struct sm_obuf_struct {
	int writable;
	int offset;
	char buf[0];
};

void usage(char *program_name)
{
    printf("usage: %s: [OPTION] -s [STRING] \n", program_name);
    fputs (("\
             -c           add carriage ret    \n\
             -d           open debug          \n\
             -s [string]  send string         \n\
             -h           display help info   \n\
"), stdout);
    exit(0);
}


LPCTSTR create_sm(char *sm_name, int size)
{
    HANDLE hMapFile;
    LPCTSTR pBuf;

    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,    // use paging file
        NULL,                    // default security
        PAGE_READWRITE,          // read/write access
        0,                       // maximum object size (high-order DWORD)
        size,                	// maximum object size (low-order DWORD)
        sm_name);                 // name of mapping object

    pBuf = (char *) MapViewOfFile(hMapFile,   // handle to map object
        FILE_MAP_ALL_ACCESS, // read/write permission
        0,
        0,
        size);
	
	assert(pBuf != NULL);
	return pBuf;
}

LPCTSTR get_sm(char *sm_name, int sm_size)
{
    HANDLE hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, sm_name);
    LPCTSTR pMapView = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sm_size);
    return pMapView;
}

int main(int argc, char **argv)
{
    struct sm_obuf_struct *posm;
    
    printf("enter!\n");
    volatile char *ibuf, *obuf;
    int i;
    int len = 0;
    int c = 0;
    int car_ret = 0;
    int debug = 0;
    char *s;

    ibuf = (char *)get_sm(sm_ibuf_name, SM_INPUT_BUF_SIZE);
    obuf = (char *)get_sm(sm_obuf_name, SM_OUTPUT_BUF_SIZE);

    posm = (struct sm_obuf_struct *)obuf;
    
    if (argc == 1) {
	usage(argv[0]);
	return 0;
    }

    while ((c = getopt(argc, argv, "cds:")) != -1) {
	switch (c) {
	    case ('c'):	/* carriage ret */
	    	car_ret = 1;
		break;
	    case ('d'):
	    	debug = 1;
		break;
	    case ('s'):
	    	s = optarg;
	    	break;
	    default:
	    	usage(argv[0]);
	    	return 0;
	    	break;
	
	}
    }


    printf("ibuf: %x; obuf: %x\n", ibuf, obuf);
    //printf("size: %d\n", sizeof(struct sm_obuf_struct));
    assert(ibuf != NULL);
    assert(obuf != NULL);
    
    if (s != NULL) {
        printf("send [%s] \n", s);

        len = strlen(s);
        memcpy((void*)&ibuf[1], s, len);

        posm->writable = 1; /* it means put output in obuf */
        posm->offset   = 0;


	if (car_ret) {
            ibuf[1 + len + 0] = '\r';
	    len++;
	}

	if (debug) {
		for(i = 0; i < len; i++) {
			printf("[%d]: (%c)(0x%02x)\n", i, ibuf[1+i], ibuf[1+i]);
		}
	}
#if 0
        ibuf[1 + len + 1] = '\n';
#endif

        ibuf[0] = 1; /* it means the cmd is ready */

    }
    while (ibuf[0] == 1); /* 1. wait cmd send */
    posm->writable = 0;
#if 0
    sleep(1);             /* 2. wait 2s for output */
    posm->writable = 0;    /* let putty stop record output to obuf */
    for(i = 0; i < SM_OUTPUT_BUF_SIZE - sizeof(struct sm_obuf_struct); i++) {
        if (posm->buf[i] == 0) {
            break;
        } else {
            /* printf("%d %c(%x)\n", i, obuf[i], obuf[i]); */
            printf("%c", posm->buf[i]);
        }
    }
    
    memset(obuf, 0, SM_OUTPUT_BUF_SIZE);
#endif
    UnmapViewOfFile(ibuf);
    UnmapViewOfFile(obuf);

    printf("\nexit!\n");
    return 0;

}
