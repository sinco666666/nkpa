#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
    // 没有参数，则默认为1步
    if(args == NULL) {
        cpu_exec(1); // 执行1步
        return 0;
    }
    // 有参数，获取
    int n;
    if(sscanf(args, "%d", &n) == EOF) {
        printf("please input a number as args\n");
    }
    else {
        cpu_exec(n); // 执行n步
    }
    return 0;
}

static int cmd_info(char *args) {
    char s;
    if(args==NULL){
        printf("args error in cmd_info\n");
        return 0;
    }
    int nRet=sscanf(args,"%c",&s);
    if(nRet<=0){
        printf("args error in cmd_info\n");
        return 0;
    }
    if(s=='r'){
        int i;
        for(i=0;i<8;i++)
        {
            printf("| %-6s %010x | %-4s %010x | %-3s %010x|\n", 
                  regsl[i], reg_l(i), regsw[i], reg_w(i), regsb[i], reg_b(i));
        }
        printf("| %-6s %010x |\n", "eip", cpu.eip);
        return 0;
    }
    if(s=='w'){
        printWP();
        return 0;
    }
    printf("args error in cmd info\n");
    return 0;
}

static int cmd_x(char *args) {
    if(!args){
        printf("args error in cmd_si\n");
        return 0;
    }
    char* args_end = args + strlen(args), *first_args = strtok(args, " ");
    if(!first_args){
        printf("args error in cmd_si\n");
        return 0;
    }
    char *exprs = first_args + strlen(first_args) + 1;
    if(exprs >= args_end){
        printf("args error in cmd_si\n");
        return 0;
    }
    int n = atoi(first_args);
    bool success;
    vaddr_t addr = expr(exprs, &success);
    if(success == false)
        printf("error in expr()\n");
    printf("Memory:\n");
    for(int i = 0; i < n; i++){
        printf("0x%x:", addr);
        uint32_t val = vaddr_read(addr, 4);
        uint8_t *by = (uint8_t *)&val;
        printf("0x");
        for(int j = 3; j >= 0; j--)
            printf("%02x", by[j]);
        printf("\n");
        addr += 4;
    }
    return 0;
}

static int cmd_e(char *args){
  bool success;
  int32_t result = expr(args, &success);

  if (!success) {
      printf("Error evaluating expression: %s\n", args);
  } else {
      printf("Result: %d (0x%x)\n", result, result);
  }

  return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute Ninstructions,the default number is 1",cmd_si},
  { "info", "Print register status or Watch information",cmd_info},
  { "x", "Evaluate the expression EXPR, use the result as the starting memory address, and output N consecutive 4-bytes in hexadecimal form",cmd_x},
  { "e", "expr" , cmd_e},
  
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
