#include <stdio.h>

#define GROUP_SIZE (64) /* in bytes */
//#define GROUP_SIZE (16) /* in bytes */

int main(int argc, char **argv)
{

    FILE *fp;
    char *ifile;
    int i, x, size, len, count;
    int oaddr, sram_addr, flash_addr;
    int buf[GROUP_SIZE / sizeof(int)] = {0};
    unsigned int checksum = 0;

    if (argc != 4) {
        printf("%s file sram_addr flash_addr\n", argv[0]);
        return -1;
    }

    ifile = argv[1];
    oaddr = sram_addr = strtoul(argv[2], NULL, 0);
    flash_addr = strtoul(argv[3], NULL, 0);
    printf("ifile: [%s]\n", ifile);
    printf("sram_addr  : [0x%08x]\n", sram_addr);
    printf("flash_addr : [0x%08x]\n", flash_addr);

    if ((fp = fopen(ifile, "rb+")) == NULL) {
        printf("fopen fail!\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    len = ftell(fp);

    if ((len % GROUP_SIZE) != 0) {
        printf("file size must align by %d bytes!\n", GROUP_SIZE);
        return -1;
    }

    fseek(fp, 0L, SEEK_SET);

    count = len / GROUP_SIZE;

    for(i = 0; i < count; i++) {
        fread(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), fp);
#if 0
        printf("fw 0x%08x 4 0x%08x 0x%08x 0x%08x 0x%08x\n",
                addr, buf[0], buf[1], buf[2], buf[3]);
#endif
        printf("w 0x%08x ", sram_addr);
        for(x = 0; x < sizeof(buf) / sizeof(buf[0]); x++) {
            printf("0x%08x ", buf[x]);
        }
        printf("\n");

        sram_addr += GROUP_SIZE;

        for(x = 0; x < sizeof(buf) / sizeof(buf[0]); x++) {
            checksum += buf[x];
        }
    }

    printf("cksum 0x%08x 0x%08x\n", oaddr, len / 4);

    printf("fmcpy 0x%08x 0x%08x 0x%08x\n", flash_addr, oaddr, len);
    printf("cksum 0x%08x 0x%08x\n", flash_addr, len / 4);

    printf("checksum: 0x%08x\n", checksum);
    fclose(fp);
    return 0;
}
