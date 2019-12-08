#include <stdio.h>

/* pad to x bytes */
int main(int argc, char **argv)
{

    FILE *fp;
    char *ifile;
    char paddata = '\0';
    int padsize;
    int i, size, len;
    if (argc != 3) {
        printf("%s file padsize\n", argv[0]);
        return -1;
    }

    ifile = argv[1];
    padsize = strtoul(argv[2], NULL, 0);
    printf("ifile:   [%s]\n", ifile);
    printf("padsize: [%d]\n", padsize);

    if ((fp = fopen(ifile, "r+")) == NULL) {
        printf("fopen fail!\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    len = ftell(fp);

    if (len % padsize != 0) {
        size = padsize - (len % padsize);

	printf("len: %d\n", len);
	for(i = 0; i < size; i++) {
	    fwrite(&paddata, sizeof(char), 1, fp);
	}
    }

    fclose(fp);
    return 0;
}
