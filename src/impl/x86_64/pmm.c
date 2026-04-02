#include "x86_64/pmm.h"

#define PAGE_SIZE 4096

static uint32_t *pmm_bitmap;
static size_t    pmm_total_pages;

#define BIT_SET(i)   (pmm_bitmap[(i)/32] |=  (1 << ((i)%32)))
#define BIT_CLEAR(i) (pmm_bitmap[(i)/32] &= ~(1 << ((i)%32)))
#define BIT_TEST(i)  (pmm_bitmap[(i)/32] &   (1 << ((i)%32)))

void pmm_init(uint64_t mem_size, void *bitmap_addr) {
    pmm_total_pages = mem_size / PAGE_SIZE;
    pmm_bitmap      = (uint32_t *)bitmap_addr;

    // Mark everything as used to start
    for (size_t i = 0; i < pmm_total_pages / 32; i++)
        pmm_bitmap[i] = 0xFFFFFFFF;
}

void pmm_free_region(uint64_t base, uint64_t size) {
    uint64_t page  = base / PAGE_SIZE;
    uint64_t count = size / PAGE_SIZE;
    for (uint64_t i = page; i < page + count; i++)
        BIT_CLEAR(i);
}

void pmm_reserve_region(uint64_t base, uint64_t size) {
    uint64_t page  = base / PAGE_SIZE;
    uint64_t count = size / PAGE_SIZE;
    for (uint64_t i = page; i < page + count; i++)
        BIT_SET(i);
}

uint64_t pmm_alloc_page() {
    for (size_t i = 0; i < pmm_total_pages; i++) {
        if (!BIT_TEST(i)) {
            BIT_SET(i);
            return (uint64_t)i * PAGE_SIZE;
        }
    }
    return 0; // out of memory
}

void pmm_free_page(uint64_t addr) {
    BIT_CLEAR(addr / PAGE_SIZE);
}
