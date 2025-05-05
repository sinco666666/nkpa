#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //依次将EFLAGS, CS, EIP寄存器的值压入堆栈
  rtl_push(&cpu.eflags.val);
  cpu.eflags.IF = 0;  
  rtl_push(&cpu.cs);
  rtl_push(&ret_addr);

  //从IDTR中读出IDT的首地址
  uint32_t idtr_base = cpu.idtr.base;

  //根据异常(中断)号在IDT中进行索引, 找到一个门描述符
  uint32_t desc_low = vaddr_read(idtr_base + NO * 8, 4);
  uint32_t desc_high = vaddr_read(idtr_base + NO * 8 + 4, 4);

  //将门描述符中的offset域组合成目标地址
  uint32_t offset = (desc_high & 0xFFFF0000) | (desc_low & 0x0000FFFF);

  //跳转到目标地址
  decoding.jmp_eip = offset;
  decoding.is_jmp = 1;

}

void dev_raise_intr() {
  cpu.INTR = true;
}
