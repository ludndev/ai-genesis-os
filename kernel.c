// Genesis OS v0.0.1 Kernel

#define VIDEO_MEMORY 0xb8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define WHITE_ON_BLACK 0x0f
#define GREEN_ON_BLACK 0x0a

// Global cursor position
int cursor_x = 0;
int cursor_y = 0;

// Port I/O
unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void port_byte_out(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

// String functions
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, int n) {
    while (n > 0 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Screen functions
void update_cursor() {
    unsigned short pos = cursor_y * SCREEN_WIDTH + cursor_x;
    port_byte_out(0x3D4, 0x0F);
    port_byte_out(0x3D5, (unsigned char)(pos & 0xFF));
    port_byte_out(0x3D4, 0x0E);
    port_byte_out(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void scroll() {
    if (cursor_y >= SCREEN_HEIGHT) {
        char* vidmem = (char*)VIDEO_MEMORY;
        for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i++) {
            vidmem[i] = vidmem[i + SCREEN_WIDTH * 2];
        }
        for (int i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i < SCREEN_HEIGHT * SCREEN_WIDTH * 2; i += 2) {
            vidmem[i] = 0;
            vidmem[i+1] = WHITE_ON_BLACK;
        }
        cursor_y = SCREEN_HEIGHT - 1;
    }
}

void print_char(char c) {
    char* vidmem = (char*)VIDEO_MEMORY;
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vidmem[(cursor_y * SCREEN_WIDTH + cursor_x) * 2] = 0;
            vidmem[(cursor_y * SCREEN_WIDTH + cursor_x) * 2 + 1] = WHITE_ON_BLACK;
        }
    } else {
        int offset = (cursor_y * SCREEN_WIDTH + cursor_x) * 2;
        vidmem[offset] = c;
        vidmem[offset+1] = WHITE_ON_BLACK;
        cursor_x++;
        if (cursor_x >= SCREEN_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    scroll();
    update_cursor();
}

void print_string(const char* str) {
    int i = 0;
    while (str[i] != 0) {
        print_char(str[i]);
        i++;
    }
}

void clear_screen() {
    char* vidmem = (char*)VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i++) {
        vidmem[i] = 0;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

// Keyboard
int shift_pressed = 0;

char scancode_to_ascii(unsigned char scancode) {
    char map[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };

    char shift_map[128] = {
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
    };

    if (scancode < 128) {
        if (shift_pressed) {
            return shift_map[scancode];
        }
        return map[scancode];
    }
    return 0;
}

void reboot() {
    print_string("Rebooting...\n");
    // Pulse the CPU reset line via the keyboard controller
    unsigned char good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xFE);
    // If that fails, try a triple fault (not implemented here, usually 0xFE is enough)
    __asm__("hlt");
}

void halt() {
    print_string("System Halted. You can now turn off your computer.\n");
    while (1) {
        __asm__("hlt");
    }
}

void delay(int count) {
    volatile int i = 0;
    while (i < count) {
        i++;
    }
}

void terminal_loop() {
    char buffer[256];
    int buf_idx = 0;
    
    print_string("\nGenesis> ");

    while (1) {
        if (port_byte_in(0x64) & 1) { // Data available
            unsigned char scancode = port_byte_in(0x60);
            
            // Handle Shift Keys
            if (scancode == 0x2A || scancode == 0x36) { // Left or Right Shift Pressed
                shift_pressed = 1;
                continue;
            } else if (scancode == 0xAA || scancode == 0xB6) { // Left or Right Shift Released
                shift_pressed = 0;
                continue;
            }

            if (scancode & 0x80) {
                // Key release, ignore (except shift handled above)
            } else {
                char c = scancode_to_ascii(scancode);
                if (c != 0) {
                    if (c == '\n') {
                        print_char('\n');
                        buffer[buf_idx] = 0;
                        
                        // Process command
                        if (strcmp(buffer, "clear") == 0) {
                            clear_screen();
                        } else if (strncmp(buffer, "echo ", 5) == 0) {
                            print_string(buffer + 5);
                            print_char('\n');
                        } else if (strcmp(buffer, "help") == 0) {
                            print_string("Available commands:\n");
                            print_string("  help    - Show this list\n");
                            print_string("  clear   - Clear the screen\n");
                            print_string("  echo    - Print text\n");
                            print_string("  reboot  - Restart the system\n");
                            print_string("  halt    - Stop the system\n");
                        } else if (strcmp(buffer, "reboot") == 0) {
                            reboot();
                        } else if (strcmp(buffer, "halt") == 0) {
                            halt();
                        } else if (buf_idx > 0) {
                            print_string("Unknown command: ");
                            print_string(buffer);
                            print_char('\n');
                        }

                        buf_idx = 0;
                        print_string("Genesis> ");
                    } else if (c == '\b') {
                        if (buf_idx > 0) {
                            print_char('\b');
                            buf_idx--;
                        }
                    } else {
                        if (buf_idx < 255) {
                            buffer[buf_idx++] = c;
                            print_char(c);
                        }
                    }
                }
            }
        }
    }
}

void kernel_main() {
    clear_screen();

    const char* message = "Genesis OS v0.0.1";
    int len = 0;
    while (message[len] != 0) len++;

    int col = (SCREEN_WIDTH - len) / 2;
    int row = SCREEN_HEIGHT / 2;
    int offset = (row * SCREEN_WIDTH + col) * 2;
    char* video_memory = (char*)VIDEO_MEMORY;

    // Print centered message
    for (int i = 0; i < len; i++) {
        *(video_memory + offset + i * 2) = message[i];
        *(video_memory + offset + i * 2 + 1) = GREEN_ON_BLACK;
    }

    // Animation "..."
    int anim_offset = offset + len * 2 + 2;
    for (int k = 0; k < 3; k++) { 
        for (int j = 0; j < 3; j++) {
             *(video_memory + anim_offset + j * 2) = ' ';
             *(video_memory + anim_offset + j * 2 + 1) = GREEN_ON_BLACK;
        }
        delay(30000000);
        for (int j = 0; j < 3; j++) {
            *(video_memory + anim_offset + j * 2) = '.';
            *(video_memory + anim_offset + j * 2 + 1) = GREEN_ON_BLACK;
            delay(30000000);
        }
    }

    // Start Terminal
    clear_screen();
    print_string("Welcome to Genesis OS v0.0.1\n");
    print_string("Type 'echo <text>' to print text.\n");
    terminal_loop();
}
