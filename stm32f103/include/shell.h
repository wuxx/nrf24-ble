#ifndef __SHELL_H__
#define __SHELL_H__

#include "stm32f10x.h"

typedef int32_t (*func_0)();
typedef int32_t (*func_4)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

#define SHELL_ARGS_MAX  (32)
struct shell_cmd_info {
    char *name;
    func_0 func;
    char *desc;
};

extern volatile char * volatile shell_cmd;

extern uint32_t argc;
extern char *argv[SHELL_ARGS_MAX];

int32_t shell(char *cmd);
void shell_init(void);

#endif /* __SHELL_H__ */
