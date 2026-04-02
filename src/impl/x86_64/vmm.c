#include "x86_64/vmm.h"

// Extract page table indices from a virtual address
#define PML4_IDX(v)  (((v) >> 39) & 0x1FF)
#define PDPT_IDX(v)  (((v) >> 30) & 0x1FF)
#define PD_IDX(v)    (((v) >> 21) & 0x1FF)
#define PT_IDX(v)    (((v) >> 12) & 0x1FF)

#define PAGE_MASK    (~(uint64_t)0xFFF)

// Get the next level table, allocating it if it doesn't exist
static page_table_t get_or_create(page_table_t table, uint64_t idx, uint64_t flags) {
    if (!(table[idx] & VMM_PRESENT)) {
        uint64_t phys = pmm_alloc_page();

        // Zero out the new table
        uint64_t *new_table = (uint64_t *)phys;
        for (int i = 0; i < 512; i++)
            new_table[i] = 0;

        table[idx] = phys | flags;
    }
    return (page_table_t)(table[idx] & PAGE_MASK);
}

void vmm_map(page_table_t pml4, uint64_t virt, uint64_t phys, uint64_t flags) {
    page_table_t pdpt = get_or_create(pml4, PML4_IDX(virt), VMM_PRESENT | VMM_WRITABLE);
    page_table_t pd   = get_or_create(pdpt, PDPT_IDX(virt), VMM_PRESENT | VMM_WRITABLE);
    page_table_t pt   = get_or_create(pd,   PD_IDX(virt),   VMM_PRESENT | VMM_WRITABLE);

    pt[PT_IDX(virt)] = phys | flags;
}

void vmm_unmap(page_table_t pml4, uint64_t virt) {
    page_table_t pdpt = (page_table_t)(pml4[PML4_IDX(virt)] & PAGE_MASK);
    if (!pdpt) return;
    page_table_t pd = (page_table_t)(pdpt[PDPT_IDX(virt)] & PAGE_MASK);
    if (!pd) return;
    page_table_t pt = (page_table_t)(pd[PD_IDX(virt)] & PAGE_MASK);
    if (!pt) return;

    pt[PT_IDX(virt)] = 0;

    // Flush TLB for this address
    __asm__ volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

uint64_t vmm_get_phys(page_table_t pml4, uint64_t virt) {
    page_table_t pdpt = (page_table_t)(pml4[PML4_IDX(virt)] & PAGE_MASK);
    if (!pdpt) return 0;
    page_table_t pd = (page_table_t)(pdpt[PDPT_IDX(virt)] & PAGE_MASK);
    if (!pd) return 0;
    page_table_t pt = (page_table_t)(pd[PD_IDX(virt)] & PAGE_MASK);
    if (!pt) return 0;

    return pt[PT_IDX(virt)] & PAGE_MASK;
}

void vmm_init() {
    // Allocate a new PML4 from the PMM
    page_table_t pml4 = (page_table_t)pmm_alloc_page();
    for (int i = 0; i < 512; i++)
        pml4[i] = 0;

    // Identity map the first 4GB so the kernel keeps working
    // (same mapping your boot asm already set up)
    for (uint64_t phys = 0; phys < 0x100000000ULL; phys += 0x1000)
        vmm_map(pml4, phys, phys, VMM_PRESENT | VMM_WRITABLE);

    // Load the new PML4 into CR3
    __asm__ volatile("mov %0, %%cr3" :: "r"((uint64_t)pml4) : "memory");
}
