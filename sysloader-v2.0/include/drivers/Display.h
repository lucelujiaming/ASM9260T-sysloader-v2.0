#ifndef	_DISPLAY_H_
#define	_DISPLAY_H_

//#define ROW_SCREEN         320
//#define COL_SCREEN         240
//#define BYTES               2
//#define ENG_ROW             8
//#define ENG_COL             16
//#define BYTES_ROW           (ROW_SCREEN/ENG_ROW) 
//#define BYTES_COL           (COL_SCREEN/ENG_COL)
//#define COLOUR1              0x00
//#define COLOUR2              0xff


extern const unsigned char FontAsc[];
extern const unsigned char FontEng[];

void draw_p(long row,long col);
int draw_row_line(long row);
int draw_col_line(long col);
void draw_p(long row,long col);
int draw_row_line(long row);
int draw_col_line(long col);
int draw_asc(long row,long col,long eng);
int lcd_printf(long row,long col,char *idata);

/*void	DisplayInit(void);
void	DisplayUpdate(U8 cColumn,U8 cPage,U8 cWidth,U8 cHeight);
void	DisplayClearRange(U8 cColumn,U8 cPage,U8 cWidth,U8 cHeight);
void	DisplayPrintEng(U8 cColumn,U8 cPage,U8* pString);
void	DisplayPrintAsc(U8 cColumn,U8 cPage,U8* pString);
void	DisplayPrintHex(U8 cColumn,U8 cPage, U8 cData);

void	DisplayDispEng(U8 cColumn,U8 cPage,U8 cData);
void	DisplayDispAsc(U8 cColumn,U8 cPage,U8 cData);
*/
#endif	// _DISPLAY_H_
