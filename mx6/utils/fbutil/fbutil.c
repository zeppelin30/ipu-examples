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

static int print_usage (void) {
    printf("\nfbutil [option] [fb #] [value]\n");
    printf("[option] fbutil \t\t [0-255] - 0:transparent \n");
    printf("            colorkey \t\t colorkey value\n");
    printf("ex) fbutil alpha 1 100\n");
    printf("    fbutil colorkey 1 on 100\n");
    printf("    fbutil colorkey 1 off\n\n");

    return 0;
}

int main (int argc, char *argv[]) {
    struct fb_var_screeninfo fb0_var;
    struct fb_fix_screeninfo fb0_fix;
    int fd_fb0;
    struct mxcfb_gbl_alpha g_alpha;
    struct mxcfb_color_key ckey;
    char fb_name[]="/dev/fb";
    char option[64];

    if (argc < 3) {
        print_usage();
        exit(-1);
    }

    strcpy(option,argv[1]);

    /*check option*/
    if ( !(strcmp(option,"alpha") )&& !(strcmp(option,"colorkey") ) ){
        printf("Unsupported option:%s\n", option);
        exit(-1);
    }
    /*check framebuffer number*/
    if (atoi(argv[2]) < 0 || atoi(argv[2]) > NUM_FB) {
        print_usage();
        exit(-1);
    }

    strcat(fb_name, argv[2]);

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

    if(!strcmp(option, "alpha") ) {
        /* Enable global alpha */
        if(! argv[3]) {
            printf("Invalid alpha value\n");
            close(fd_fb0);
            goto done;
        }
        g_alpha.alpha = atoi(argv[3]);
        g_alpha.enable = 1;

        if( g_alpha.alpha > MAX_ALPHA)
            g_alpha.alpha = MAX_ALPHA;
        else if( g_alpha.alpha < 0)
            g_alpha.alpha = 0;
        printf("Global alpha changed as %d\n", g_alpha.alpha);

        if (ioctl(fd_fb0, MXCFB_SET_GBL_ALPHA, &g_alpha) < 0) {
            printf("Set global alpha failed\n");
            close(fd_fb0);
            goto done;
        }
    }
    else if(!strcmp(option, "colorkey") ) {
        if (!strcmp(argv[3], "on") ) {
            ckey.enable = 1;
            if ((argv[4])) {
                ckey.color_key = atoi(argv[4]);
                printf("Colorkey: %d\n", ckey.color_key);
            }
            else {
                printf("Need colorkey value\n");
                close(fd_fb0);
                goto done;
            }

        }
        else if (!strcmp(argv[3], "off") ) {
            ckey.enable = 0;
            ckey.color_key = 0x0;
        }
        else {
            print_usage();
            close(fd_fb0);
            goto done;
        }
        /* Enable/Disalbe color key  */
        printf("Color key enabled:%d\n", ckey.enable);
        if (ioctl(fd_fb0,MXCFB_SET_CLR_KEY,&ckey) < 0) {
            printf( "MXCFB_SET_CLR_KEY failed ");
            close(fd_fb0);
            goto done;
        }
    }

    close(fd_fb0);
    return 0;

done:
    return -1;
}
