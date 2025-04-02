#include "cpu/exec.h"

make_EHelper(mov);

// control.c
make_EHelper(call);
make_EHelper(ret); 
make_EHelper(jcc); 
make_EHelper(jmp); 
make_EHelper(call_rm);
make_EHelper(jmp_rm);

make_EHelper(call);
make_EHelper(sub);
make_EHelper(xor);
make_EHelper(push);
make_EHelper(pop);
make_EHelper(ret);
make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);


make_EHelper(lea);
make_EHelper(and);
make_EHelper(nop);