#include <string.h>
#include "shell.h"
#include "log.h"
#include "systest.h"

#if 0
s32 test_cpu_all(u32 argc, char **argv);
s32 test_timer_all(u32 argc, char **argv);
s32 test_gpio_all(u32 argc, char **argv);
s32 test_libc_all(u32 argc, char **argv);
s32 test_log_all(u32 argc, char **argv);
s32 test_os_all(u32 argc, char **argv);
s32 test_wdt_all(u32 argc, char **argv);
#endif
int32_t test_gpio_all(void);
int32_t test_timer_all(void);
int32_t test_flash_all(void);
int32_t test_dma_all(void);
int32_t test_log_all(void);

int32_t sub_cmd_help(void);

struct shell_cmd_info sub_ci[] =  {
    {  "gpio",  test_gpio_all,  ""},
#if 0		
    {  "timer", test_timer_all, ""},
    {  "flash", test_flash_all, ""},
    {  "dma",   test_dma_all,   ""},
    {  "log",   test_log_all,   ""},
#endif
#if 0	
    {  "cpu",   test_cpu_all,   ""},
    {  "timer", test_timer_all, ""},
    {  "gpio",  test_gpio_all,  ""},
    {  "libc",  test_libc_all,  ""},
    {  "log",   test_log_all,   ""},
    {  "os",    test_os_all,    ""},
    {  "wdt",   test_wdt_all,   ""},
    {  "help",  sub_cmd_help,   ""},
#endif
};

void dump_mem(uint32_t addr, uint32_t word_nr)
{
    uint32_t i;
    uint32_t *p = (uint32_t *)addr;
    for(i = 0; i < word_nr; i++) {
        if (i % 4 == 0) {
            PRINT_EMG("\n[0x%X]: ", &p[i]);
        }
        PRINT_EMG("0x%X ", p[i]);
    }

    PRINT_EMG("\n");
}

static int32_t sub_cmd_help()
{
    uint32_t i;
    for(i = 0; i < (sizeof(sub_ci)/sizeof(sub_ci[0])); i++) {
        PRINT_EMG("%s:\t\t\t%s\n", sub_ci[i].name, sub_ci[i].desc);
    }
    return 0;
}

static int32_t get_cmd_index(char *cmd)
{
    uint32_t i;
    for(i = 0; i < (sizeof(sub_ci)/sizeof(sub_ci[0])); i++) {
        if (strcmp(sub_ci[i].name, cmd) == 0) {
            return i;
        }
    }
    return -1;
}

int32_t systest(uint32_t argc, char **argv)
{
    int32_t i;
    int32_t ret;

    if ((i = get_cmd_index(argv[1])) == -1) {
        PRINT_EMG("illegal sub-cmd [%s]\n", argv[1]);
        sub_cmd_help();
        return -1;
    }

    ret = sub_ci[i].func();
    return ret;
}

