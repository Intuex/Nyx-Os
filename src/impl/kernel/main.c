#include "print.h"
#include "keyboard.h"
#include "x86_64/rtc.h"
#include "x86_64/pmm.h"

typedef struct {
    uint32_t size;
    uint64_t base_addr;   // must be 64-bit for multiboot mmap
    uint64_t length;      // must be 64-bit for multiboot mmap
    uint32_t type;
} __attribute__((packed)) mmap_entry_t;  // fixed: __atribute__ → __attribute__

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint8_t  pad[8];
    uint32_t mmap_length; // added: was missing
    uint32_t mmap_addr;
} __attribute__((packed)) multiboot_info_t;

int shift_pressed = 0;
int caps_lock = 0;

#define KEY_CODE_CAPSLOCK 0x3A
#define KEY_CODE_LSHIFT 0x2A
#define KEY_CODE_RSHIFT 0x36
#define KEY_CODE_A 0x1E
#define KEY_CODE_B 0x30
#define KEY_CODE_C 0x2E
#define KEY_CODE_D 0x20
#define KEY_CODE_E 0x12
#define KEY_CODE_F 0x21
#define KEY_CODE_G 0x22
#define KEY_CODE_H 0x23
#define KEY_CODE_I 0x17
#define KEY_CODE_J 0x24
#define KEY_CODE_K 0x25
#define KEY_CODE_L 0x26
#define KEY_CODE_M 0x32
#define KEY_CODE_N 0x31
#define KEY_CODE_O 0x18
#define KEY_CODE_P 0x19
#define KEY_CODE_Q 0x10
#define KEY_CODE_R 0x13
#define KEY_CODE_S 0x1F
#define KEY_CODE_T 0x14
#define KEY_CODE_U 0x16
#define KEY_CODE_V 0x2F
#define KEY_CODE_W 0x11
#define KEY_CODE_X 0x2D
#define KEY_CODE_Y 0x15
#define KEY_CODE_Z 0x2C
#define KEY_CODE_SPACE 0x39
#define KEY_CODE_ENTER 0x1C

char to_ascii(uint16_t code) {
    char c = 0;

    switch (code) {
        case KEY_CODE_A: c = 'a'; break;
        case KEY_CODE_B: c = 'b'; break;
        case KEY_CODE_C: c = 'c'; break;
        case KEY_CODE_D: c = 'd'; break;
        case KEY_CODE_E: c = 'e'; break;
        case KEY_CODE_F: c = 'f'; break;
        case KEY_CODE_G: c = 'g'; break;
        case KEY_CODE_H: c = 'h'; break;
        case KEY_CODE_I: c = 'i'; break;
        case KEY_CODE_J: c = 'j'; break;
        case KEY_CODE_K: c = 'k'; break;
        case KEY_CODE_L: c = 'l'; break;
        case KEY_CODE_M: c = 'm'; break;
        case KEY_CODE_N: c = 'n'; break;
        case KEY_CODE_O: c = 'o'; break;
        case KEY_CODE_P: c = 'p'; break;
        case KEY_CODE_Q: c = 'q'; break;
        case KEY_CODE_R: c = 'r'; break;
        case KEY_CODE_S: c = 's'; break;
        case KEY_CODE_T: c = 't'; break;
        case KEY_CODE_U: c = 'u'; break;
        case KEY_CODE_V: c = 'v'; break;
        case KEY_CODE_W: c = 'w'; break;
        case KEY_CODE_X: c = 'x'; break;
        case KEY_CODE_Y: c = 'y'; break;
        case KEY_CODE_Z: c = 'z'; break;
        case KEY_CODE_SPACE: return ' ';
        case KEY_CODE_ENTER: return '\n';
        case KEY_CODE_CAPSLOCK: return 0;
    }

    if (c >= 'a' && c <= 'z') {
        if (shift_pressed ^ caps_lock) {
            c -= 32;
        }
    }

    return c;
}

void handle_input(struct KeyboardEvent event) {
    if (event.code == KEY_CODE_LSHIFT || event.code == KEY_CODE_RSHIFT) {
        if (event.type == KEYBOARD_EVENT_TYPE_MAKE) {
            shift_pressed = 1;
        } else if (event.type == KEYBOARD_EVENT_TYPE_BREAK) {
            shift_pressed = 0;
        }
        return;
    }

    if (event.code == KEY_CODE_CAPSLOCK) {
        if (event.type == KEYBOARD_EVENT_TYPE_MAKE) {
            caps_lock = !caps_lock;
        }
        return;
    }

    if (event.type == KEYBOARD_EVENT_TYPE_MAKE) {
        char c = to_ascii(event.code);
        if (c) {
            print_set_color(PRINT_COLOR_BLUE, PRINT_COLOR_WHITE);
            print_char(c);
        }
    }
}

void kernel_main(multiboot_info_t *mb) {  // fixed: mb must be a parameter
    extern uint8_t _kernel_end;

    uint64_t total_ram = ((uint64_t)mb->mem_upper + 1024) * 1024;

    pmm_init(total_ram, (void *)&_kernel_end);

    // Walk the memory maph
    mmap_entry_t *mmap = (mmap_entry_t *)(uint64_t)mb->mmap_addr;
    mmap_entry_t *end  = (mmap_entry_t *)((uint64_t)mb->mmap_addr + mb->mmap_length);

    while (mmap < end) {
        if (mmap->type == 1)
            pmm_free_region(mmap->base_addr, mmap->length);  // fixed spelling
        mmap = (mmap_entry_t *)((uint8_t *)mmap + mmap->size + sizeof(mmap->size));
    }

    // Reserve the kernel itself so it never gets allocated over
    uint64_t kernel_start = 0x100000;
    uint64_t kernel_size  = (uint64_t)&_kernel_end - kernel_start;
    pmm_reserve_region(kernel_start, kernel_size);  // fixed spelling

    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("");






    print_str("Welcome to our NyxOs!\n");

    keyboard_init();
    keyboard_set_handler(handle_input);

    while (1);
}
