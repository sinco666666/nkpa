#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_EQ,
  TK_REGISTER,
  TK_NUMBER,
  TK_NE,
  TK_AND,
  TK_OR

  /* TODO: Add more token types */

};

/* 运算符优先级 */
enum {
    OP_LV0 = 0 ,   // number, register
    OP_LV1 = 10 ,  // ( )
    OP_LV2_1 = 21, // unary + , -
    OP_LV2_2 = 22, // dereference *
    OP_LV3 = 30,   // * , / , %
    OP_LV4 = 40,   // + , -
    OP_LV7 = 70,   // == , !=
    OP_LV11 = 110, // &&
    OP_LV12 = 120  // ||
};

static struct rule {
  char *regex;
  int token_type;
  int precedence; 
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE, 0},    // spaces
  {"\\+", '+', OP_LV4},         // plus
  {"\\-", '-', OP_LV4},
  {"\\*", '*', OP_LV3},
  {"/", '/', OP_LV3},
  {"\\(", '(', OP_LV1},
  {"\\)", ')', OP_LV1},
  {"\\$[a-zA-Z0-9]+", TK_REGISTER, OP_LV0},
  {"0[xX][0-9a-fA-F]+", TK_NUMBER, OP_LV0}, //hex
  {"0|[1-9][0-9]*", TK_NUMBER, OP_LV0},
  {"!=", TK_NE, OP_LV7},  
  {"&&", TK_AND, OP_LV11},
  {"\\|\\|", TK_OR, OP_LV12},
  {"==", TK_EQ, OP_LV7}         // equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
  int precedence; 
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case '+':
          case '-':
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV4;
            break;

          case '*':
          case '/':
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV3;
            break;

          case TK_EQ:
          case TK_NE:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV7;
            break;

          case TK_AND:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV11;
            break;

          case TK_OR:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV12;
            break;

          default:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = rules[i].precedence; // 直接使用默认优先级
            break;
        }

        if (substr_len < sizeof(tokens[nr_token].str)) {
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
        } else {
          printf("Token too long at position %d\n", position);
          return false;
        }

        ++nr_token
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

/* 检查是否是完整匹配的括号 */
bool check_parentheses(int p, int q) {
  if (tokens[p].type != '(' || tokens[q].type != ')') return false;

  int balance = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') balance++;
    if (tokens[i].type == ')') balance--;
    if (balance == 0 && i < q) return false;  // 遇到非最外层匹配
  }

  return balance == 0;
}

/* 找到主导运算符 */
int find_dominant_operator(int p, int q) {
  int min_precedence = 9999, op_pos = -1, balance = 0;

  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      balance++;
    } else if (tokens[i].type == ')') {
      balance--;
    } else if (balance == 0) {  // 只有最外层的运算符才作为主导运算符
      if (tokens[i].precedence <= min_precedence) {
        min_precedence = tokens[i].precedence;
        op_pos = i;
      }
    }
  }

  return op_pos;
}


/* 递归求值 */
uint32_t eval(int p, int q, bool *success) {
  if (p > q) {
    *success = false;
    return 0;
  } else if (p == q) {
    if (tokens[p].type == TK_NUMBER) {
      return strtol(tokens[p].str, NULL, 0);
    } else {
      *success = false;
      return 0;
    }
  } else if (check_parentheses(p, q)) {
    return eval(p + 1, q - 1, success);
  } else {
    int op = find_dominant_operator(p, q);
    if (op == -1) {
      *success = false;
      return 0;
    }

    uint32_t val1 = eval(p, op - 1, success);
    uint32_t val2 = eval(op + 1, q, success);

    if (!(*success)) return 0;

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/':
        if (val2 == 0) {
          printf("Error: Division by zero\n");
          *success = false;
          return 0;
        }
        return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_NE: return val1 != val2;
      case TK_AND: return val1 && val2;
      case TK_OR: return val1 || val2;
      default:
        *success = false;
        return 0;
    }
  }
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  return eval(0, nr_token - 1, success);

  return 0;
}
