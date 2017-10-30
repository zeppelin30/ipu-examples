#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/mxcfb.h>
#include <linux/ipu.h>

#define MAX_ALPHA 255
#define NUM_FB 4

static int print_fbinfo(struct fb_var_screeninfo fb) {
    printf("\nFB information \n");
    printf("xres = %d\n",  fb.xres);
    printf("xres_virtual = %d\n",  fb.xres_virtual);
    printf("yres = %d\n",  fb.yres);
    printf("yres_virtual = %d\n",  fb.yres_virtual);
    printf("bits_per_pixel = %d\n",  fb.bits_per_pixel);
    printf("pixclock = %d\n",  fb.pixclock);
    printf("height = %d\n",  fb.height);
    printf("width = %d\n",  fb.width);
    printf("Pixel format : RGBX_%d%d%d%d\n",fb.red.length,
                                                 fb.green.length,
                                                 fb.blue.length,
                                                 fb.transp.length);
    printf(" Begin of bitfields(Byte ordering):-\n");
    printf("  Red    : %d\n",fb.red.offset);
    printf("  Blue   : %d\n",fb.blue.offset);
    printf("  Green  : %d\n",fb.green.offset);
    printf("  Transp : %d\n",fb.transp.offset);

    return 0;
}

int main (int argc, char *argv[])
{
    struct fb_var_screeninfo fb0_var;
    struct fb_fix_screeninfo fb0_fix;
    int fd_fb0;
    struct mxcfb_gbl_alpha g_alpha;
    char fb_name[]="/dev/fb";

    if (argc < 3) {
        printf(" alhpa [fb #] [0 - 255] - 0: transparent \n");
        printf(" ex,) alhpa 1 100\n");
        exit(-1);
    }

    if (atoi(argv[1]) < 0 || atoi(argv[1]) > NUM_FB) {
        printf(" Wrong number of framebuffer /dev/fb%d\n",atoi(argv[1]) );
        exit(-1);
    }

    strcat(fb_name, argv[1]);

    printf("Opening %s\n",fb_name);
    // Open Framebuffer and gets its address
    if ((fd_fb0 = open(fb_name, O_RDWR, 0)) < 0) {
        printf("Unable to open %s\n",fb_name);
        goto done;
    }

    if ( ioctl(fd_fb0, FBIOGET_FSCREENINFO, &fb0_fix) < 0) {
        printf("Get FB fix info failed!\n");
        close(fd_fb0);
        goto done;
    }

    if ( ioctl(fd_fb0, FBIOGET_VSCREENINFO, &fb0_var) < 0) {
        printf("Get FB var info failed!\n");
        close(fd_fb0);
        goto done;
    }

    print_fbinfo(fb0_var);

    /* Enable global alpha */
    g_alpha.alpha = atoi(argv[2]);
    g_alpha.enable = 1;

    if( g_alpha.alpha > MAX_ALPHA)
        g_alpha.alpha = MAX_ALPHA;
    else if( g_alpha.alpha < 0)
        g_alpha.alpha = 0;
    
    if (ioctl(fd_fb0, MXCFB_SET_GBL_ALPHA, &g_alpha) < 0) {
        printf("Set global alpha failed\n");
        close(fd_fb0);
        goto done;
    }

    close(fd_fb0);
    return 0;

done:
    return -1;
}
