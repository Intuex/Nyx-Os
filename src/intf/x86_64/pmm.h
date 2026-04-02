#pragma once
#include <stdint.h>
#include <stddef.h>

void     pmm_init(uint64_t mem_size, void *bitmap_addr);
void     pmm_free_region(uint64_t base, uint64_t size);
void     pmm_reserve_region(uint64_t base, uint64_t size);
uint64_t pmm_alloc_page();
void     pmm_free_page(uint64_t addr);
