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

#define NUM_FB 4

static int print_fbinfo (struct fb_var_screeninfo fb) {
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
    printf("  Transp : %d\n\n",fb.transp.offset);

    return 0;
}

int main (int argc, char *argv[])
{
    struct fb_var_screeninfo fb0_var;
    struct fb_fix_screeninfo fb0_fix;
    int fd_fb0;
    char fb_name[]="/dev/fb";

    struct mxcfb_pos pos;
    int width, height;

    if (argc < 6) //fb, xoff, yoff, xres, yres
    {
        printf(" Put fb,x,y,w,h argument\n");
        exit(-1);
    }

    /*check framebuffer number*/
    if (atoi(argv[1]) < 0 || atoi(argv[1]) > NUM_FB) {
        printf(" Put fb,x,y,w,h argument\n");
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

    // frame position, width and height
    pos.x = atoi(argv[2]);
    pos.y = atoi(argv[3]);
    width = atoi(argv[4]);
    height = atoi(argv[5]);
    
    fb0_var.xres = fb0_var.xres_virtual = width ;
    fb0_var.yres = height ;
    fb0_var.yres_virtual = height*2 ;

    if(ioctl( fd_fb0, FBIOPUT_VSCREENINFO, &fb0_var ) < 0){
        perror( "FBIOPUT_VSCREENINFO");
        close(fd_fb0);
        return  -1;
    }
    
    if (ioctl(fd_fb0, MXCFB_SET_OVERLAY_POS, &pos) < 0) {
        printf("Set position failed\n");
        close(fd_fb0);
        goto done;
    }

    close(fd_fb0);
    return 0;

done:
    return -1;
}
