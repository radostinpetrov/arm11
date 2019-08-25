#include "fb.h"
#include "mailbox.h"

typedef struct {
    unsigned int width;       // width of the display
    unsigned int height;      // height of the display
    unsigned int virtual_width;  // width of the virtual framebuffer
    unsigned int virtual_height; // height of the virtual framebuffer
    unsigned int pitch;       // number of bytes per row
    unsigned int depth;       // number of bits per pixel
    unsigned int x_offset;    // x of the upper left corner of the virtual fb
    unsigned int y_offset;    // y of the upper left corner of the virtual fb
    unsigned int framebuffer; // pointer to the start of the framebuffer
    unsigned int size;        // number of bytes in the framebuffer
} fb_t;

// fb is volatile because the GPU will write to it
static volatile fb_t fb __attribute__ ((aligned(16)));

static unsigned _mode;

void fb_init(unsigned width, unsigned height, unsigned depth, unsigned db) {
    _mode = db;

    if(db) {
        fb.height = height;
        fb.virtual_height = 2 * height;
    } else {
      fb.height = height;
      fb.virtual_height = height;
    }

    fb.width = width;
    fb.virtual_width = width;
    fb.depth = depth;
    fb.x_offset = 0;
    fb.y_offset = 0;

    // set values returned by the GPU to 0; see mailbox documentation
    fb.pitch = 0;
    fb.framebuffer = 0;
    fb.size = 0;

    mailbox_write(MAILBOX_FRAMEBUFFER, (unsigned)&fb + GPU_NOCACHE);
    (void) mailbox_read(MAILBOX_FRAMEBUFFER);
}

void fb_clear(unsigned int size) {
    unsigned long long *im = (unsigned long long *) fb_get_draw_buffer(); 

    // Assuming buffer size in bytes is divisible by 16.
    unsigned int s = size / 2;
    for(unsigned i = 0; i < s; i += 8, im += 8) {
        im[0] = 0;
        im[1] = 0;
        im[2] = 0;
        im[3] = 0;
        im[4] = 0;
        im[5] = 0;
        im[6] = 0;
        im[7] = 0;
    }
}

unsigned fb_get_depth(void) {
    return fb.depth;
}

unsigned fb_get_height(void) {
    return fb.height;
}

unsigned fb_get_width(void) {
    return fb.width;
}

void fb_swap_buffer(void) {
    fb.y_offset = fb.y_offset ? 0 : fb.height;
    mailbox_write(MAILBOX_FRAMEBUFFER, (unsigned)&fb + GPU_NOCACHE);
    (void) mailbox_read(MAILBOX_FRAMEBUFFER);
}

unsigned char* fb_get_draw_buffer(void) {
    if(_mode)
        return fb.y_offset ? (unsigned char *) fb.framebuffer : (unsigned char *) (fb.framebuffer + fb.size / 2);
    return (unsigned char *) fb.framebuffer;
}

