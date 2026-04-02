#pragma once
#include <stdint.h>
#include "pmm.h"

//page flags
#define VMM_PRESENT (1ULL << 0)
#define VMM_WRITABLE (1ULL << 1)
#define VMM_USER (1ULL << 2)
#define VMM_HUGE (1ULL << 7)

typedef uint64_t* page_table_t;

void vmm_init();
void vmm_map(page_table_t pml4, uint64_t virt, uint64_t phys, uint64_t flags);
void vmm_unmap(page_table_t pml4, uint64_t virt);
uint64_t vmm_get_phys(page_table_t pml4, uint64_t virt);
