#ifndef __LCDIF_H__
#define __LCDIF_H__

//#include <common.h>


#ifdef CONFIG_LCD_IF

	#define AS3310_DMA_LCDIF 4
	#define LCD_MAX_XSIZE 320
	#define LCD_MAX_YSIZE 240

    #define LCDIF_ROW 		320
    #define LCDIF_COL 		240
    #define LCDIF_PKG_NUM 	4
    #define FRAME_BUFFER_LEN (LCDIF_ROW*LCDIF_COL*2)
    #define PER_BUFFER_LEN (FRAME_BUFFER_LEN/LCDIF_PKG_NUM)

    #define LCDIF_BL_PORT      3
    #define LCDIF_BL_PIN       12
/*asc display*/
    #define ROW_SCREEN         320
    #define COL_SCREEN         240
    #define BYTES               2
    #define ENG_ROW             8
    #define ENG_COL             16
    #define BYTES_ROW           (ROW_SCREEN/ENG_ROW) 
    #define BYTES_COL           (COL_SCREEN/ENG_COL)
    #define COLOUR1              0x00
    #define COLOUR2              0xff

    //extern long FRAME_BUFFER_LCDIF_ADDR;   
    //extern long FRAME_BUFFER_LCDIF_ADDR;  
    //#define LCDIF_PKG_BASE (FRAME_BUFFER_LCDIF_ADDR+(LCDIF_ROW*LCDIF_COL*2))

    extern void* lcdif_data_buffer;

    void lcd_delay(int n);
    void lcd_write_reg(unsigned short n);
    void Init_data(unsigned short index,unsigned short instruction);
    int lcdif_dma_pkg_init(void);
    int lcdif_init(void);
    void  PowerOnLcdinit(void);
    void blank_buff(int val);
    void refresh_dma(void);
    void lcdif_dma_start(void);
    void LCD_Command(unsigned int cmd);
    void LCD_Data(unsigned int data);
    void LCD_Datas(unsigned short * data,int counts);
    void LCD_static_Datas(unsigned short data,int counts);
    void LCD_Reg_Set(unsigned int cmd,unsigned int data);
    void LCD_SetDispAddr(unsigned int x,unsigned int y);
    void LCD_SetPixel(unsigned int x, unsigned int y, unsigned int color);
    void LCDDEV_SetWindow(int x0, int y0, int x1, int y1);
    int do_lcdif_init(cmd_tbl_t *cmdtp,int argc,char* argv[]);
    void fill_rgb_buffer(void);
    int lcdif_display_RGB(cmd_tbl_t *cmdtp,int argc,char* argv[]); 
    int lcdif_refresh_dma(cmd_tbl_t *cmdtp,int argc,char* argv[]); 
    void lcdif_display_color(unsigned short color);
    int lprintf(char *idata);
    int do_lcdif_display_color(cmd_tbl_t *cmdtp,int argc,char* argv[]);
    int do_lcdif_refresh_fb(cmd_tbl_t *cmdtp,int argc,char* argv[]);	
    int do_lcdif_word(cmd_tbl_t *cmdtp,int argc,char* argv[]);


    #define IS_LCIF_DMA_COMPLETE() ((inl(HW_APBX_CH4_SEMA)&0x00ff0000)==0)

#endif

#endif
