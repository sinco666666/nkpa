#include "nemu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

#define PDX(va)     (((uint32_t)(va) >> 22) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> 12) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)
#define PTE_ADDR(pte)   ((uint32_t)(pte) & ~0xfff)
paddr_t page_translate(vaddr_t addr, bool w1r0) {
    //Log("page_translate");
    PDE pde, *pgdir;
    PTE pte, *pgtab;
    if (cpu.cr0.protect_enable && cpu.cr0.paging) {
	    pgdir = (PDE *)(PTE_ADDR(cpu.cr3.val));
	    pde.val = paddr_read((paddr_t)&pgdir[PDX(addr)], 4);
	    assert(pde.present);
	    pde.accessed = 1;

	    pgtab = (PTE *)(PTE_ADDR(pde.val)); 
	    pte.val = paddr_read((paddr_t)&pgtab[PTX(addr)], 4);
	    assert(pte.present);
	    pte.accessed = 1;
	    pte.dirty = w1r0 ? 1 : pte.dirty; 

	    return PTE_ADDR(pte.val) | OFF(addr); 
	}
    return addr;
}

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  //return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  int r = is_mmio(addr);
  if (r == -1)
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  else
    return mmio_read(addr, len, r);
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  //memcpy(guest_to_host(addr), &data, len);
  int r = is_mmio(addr);
  if (r == -1)
    memcpy(guest_to_host(addr), &data, len);
  else
    mmio_write(addr, len, data, r);
}

// uint32_t vaddr_read(vaddr_t addr, int len) {
//   //return paddr_read(addr, len);
//   //Log("vaddr_read");
//   if ((((addr) + (len) - 1) & ~PAGE_MASK) != ((addr) & ~PAGE_MASK)) {
// 	uint32_t data = 0;
// 	for(int i=0;i<len;i++){
// 		paddr_t paddr = page_translate(addr + i, false);
// 		data += (paddr_read(paddr, 1))<<8*i;
// 	}
// 	return data;
//   //assert(0);
//   } else {
//     paddr_t paddr = page_translate(addr, false);
//     return paddr_read(paddr, len);
//   }
// }

// void vaddr_write(vaddr_t addr, int len, uint32_t data) {
//   //Log("vaddr_write");
//   //paddr_write(addr, len, data);
//   if ((((addr) + (len) - 1) & ~PAGE_MASK) != ((addr) & ~PAGE_MASK)) {
// 	for(int i=0;i<len;i++){
// 		paddr_t paddr = page_translate(addr + i,true);
// 		paddr_write(paddr,1,data>>8*i);
// 	}
//   //assert(0);
//   } else {
//     paddr_t paddr = page_translate(addr, true);
//     paddr_write(paddr, len, data);
//   }
// }
uint32_t vaddr_read(vaddr_t addr, int len) {
    if (PTE_ADDR(addr) != PTE_ADDR(addr + len - 1)) {
        printf("error: the data pass two pages: addr=0x%u, len=%d!\n", addr, len);
        assert(0);
    } else {
        paddr_t paddr = page_translate(addr, false);
        return paddr_read(paddr, len);
    }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
    if (PTE_ADDR(addr) != PTE_ADDR(addr + len - 1)) {
        printf("error: the data pass two pages: addr=0x%u, len=%d!\n", addr, len);
        assert(0);
    } else {
        paddr_t paddr = page_translate(addr, true);
        paddr_write(paddr, len, data);
    }
}
