// kernel.c
// Genesis OS v0.0.1 Kernel

// Genesis OS v0.0.1 Kernel

#define VIDEO_MEMORY 0xb8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define WHITE_ON_BLACK 0x0f
#define GREEN_ON_BLACK 0x0a

// Simple delay function (busy wait)
void delay(int count) {
    volatile int i = 0;
    while (i < count) {
        i++;
    }
}

void kernel_main() {
    char* video_memory = (char*) VIDEO_MEMORY;

    // Clear the screen
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i++) {
        *(video_memory + i) = 0;
    }

    const char* message = "Welcome to Genesis OS v0.0.1 - Kernel Loaded Successfully!";
    int len = 0;
    while (message[len] != 0) len++;

    int col = (SCREEN_WIDTH - len) / 2;
    int row = SCREEN_HEIGHT / 2;
    int offset = (row * SCREEN_WIDTH + col) * 2;

    // Print centered message
    for (int i = 0; i < len; i++) {
        *(video_memory + offset + i * 2) = message[i];
        *(video_memory + offset + i * 2 + 1) = GREEN_ON_BLACK;
    }

    // Animation "..."
    int anim_offset = offset + len * 2;
    // Run animation for roughly 3 seconds
    // Adjust delay count based on emulation speed, 10000000 is a rough guess for visible delay
    for (int k = 0; k < 6; k++) { 
        // Clear dots
        for (int j = 0; j < 3; j++) {
             *(video_memory + anim_offset + j * 2) = ' ';
             *(video_memory + anim_offset + j * 2 + 1) = GREEN_ON_BLACK;
        }
        delay(50000000);

        // Print dots one by one
        for (int j = 0; j < 3; j++) {
            *(video_memory + anim_offset + j * 2) = '.';
            *(video_memory + anim_offset + j * 2 + 1) = GREEN_ON_BLACK;
            delay(50000000);
        }
    }
}
