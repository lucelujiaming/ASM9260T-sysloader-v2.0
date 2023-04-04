/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader Lcd Header file


------------------- Version 1.0  ----------------------
Create File
 He Yong 2006-11-06

*/

#ifndef  __LCD_H__
#define  __LCD_H__

#include <common.h>

    /* LCD device status */
#define LCD_CONTROL_RUN             (1UL<<0)
#define LCD_CONTROL_MODE_RGB        (1UL<<1)
#define LCD_CONTROL_MODE_YUV        (1UL<<2)
#define LCD_CONTROL_MODE_CYCLE      (1UL<<2)

    /* LCD device ioctls */
#define LCD_CONTROL_IOCTL_STOP      0
#define LCD_CONTROL_IOCTL_RUN       1
#define LCD_CONTROL_IOCTL_RGB       2
#define LCD_CONTROL_IOCTL_YUV       3


#define AS3310_HCLK     90    /* 90 MHz */
#define LCD_FRAME_RATE  45    /* 30 - 60 Hz */

#if CONFIG_ILI_9320RGB
    #define SCREEN_SIZE_COL 240
    #define SCREEN_SIZE_ROW 320
    
    #if CONFIG_BOARD_AS3310E_FPGA_QQ
        #define CEN_PORT   3
        #define CEN_PIN    3
    #endif// CONFIG_BOARD_AS3310E_FPGA_QQ
    
    #if CONFIG_BOARD_AS3310_DEV
        #define CEN_PORT   3
        #define CEN_PIN    3
    #endif// BORAD_DEV
    
    #if CONFIG_BOARD_AS3310_MP4_DEMO
        #define CEN_PORT   0
        #define CEN_PIN    8
    #endif// BORAD_DEMO_RGB
    
    #define DEN_PORT   1
    #define DEN_PIN    17
#endif

#if TRULY_LCD
    #define SCREEN_SIZE_ROW 272
    #define SCREEN_SIZE_COL 480
#endif

#if SAMSUNG_35
    #define SCREEN_SIZE_ROW 240
    #define SCREEN_SIZE_COL 320
#endif

#define LCD_RESET_PORT   1
#define LCD_RESET_PIN    16

#define LCD_BL_PORT      3
#define LCD_BL_PIN       12

// #define COL 320
// #define ROW 240
#define COL SCREEN_SIZE_COL
#define ROW SCREEN_SIZE_ROW
#define LCD_PIXCLK_DIVIDER 26//((int)(AS3310_HCLK*1000000 / (LCD_FRAME_RATE*COL*ROW*1.1)))

#define LCD_PKG_NUM 5
#define TRI_STEP 5

typedef struct _point_{
    int x;  // COL
    int y;  // Row
}Point;
void delayms(int ms);
void LCD_CtrlWrite_ILI9320(ulong reg, ulong data);
void u_dma_start(ulong pkg_addr,int pkg_num);
void v_dma_start(ulong pkg_addr,int pkg_num);
void ili9320_init(void);
void lcd_fill_frame_16bpp_(ushort FrameBuffer[][COL]);
void lcd_fill_frame_16bpp(void);
int do_irq_lcd(int priv);
int init_as3310_lcd_dma_pkg(void);
void lcd_init(struct device * dev);
int do_lcd_ioctl(cmd_tbl_t * cmdtp,int argc, char *argv[]);
int lcd_ioctl(struct device * dev,unsigned int cmd,unsigned long arg);
int lcd_probe(struct device * dev);


extern volatile char NCached lcd_framebuffer[COL*ROW*2];

 /* Start the DMA, dedicated channel for LCD DMA */ 
 void lcd_dma_start(ulong pkg_phy_addr,int pkg_num);

typedef unsigned short LCDBIT ;
extern void change_color(LCDBIT *color,int *color_control);
void draw_point(char * buffer,int row,int col,int val,int depth);
void Draw_Line(long start_x,long start_y,long end_x,long end_y,LCDBIT FrameBuffer[][COL],LCDBIT *color);
void Draw_Rect(long start_x,long start_y,long end_x,long end_y,LCDBIT FrameBuffer[][COL],LCDBIT *color);
void Move_Edge(int *Ax,int *Ay,int step);
void Triangle_2(LCDBIT FrameBuffer[][COL],int tri_count,int A_step,int B_step,int C_step);
void word_display(void);
int do_cmd_lcd(cmd_tbl_t * cmdtp,int argc, char *argv[]);

    #if TEXT_LIB
void Draw_word(char * buffer,Point * start_p,HzDotStruct * word_s,LCDBIT color,int depth);
    #endif
#endif //  __LCD_H__
