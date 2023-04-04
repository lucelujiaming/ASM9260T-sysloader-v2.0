#ifndef _USB_H_
#define _USB_H_


#include <common.h>
#include <drivers/usb_protocol.h>

#define BIOS_TEST

#define USB_DMA_MODE    // ##

#define	TRUE		1
#define	FALSE		0

#if CONFIG_USB_DEBUG
#define USB_DEBUG   // ##
#endif

#define NUM_STRING          (0x04)

#define EP0_PACKET_SIZE			64
//#define CONFIG_PACKET_SIZE      6 // default 512

// got from menuconfig

#ifdef      CONFIG_USB_PACKET_64
#define     CONFIG_PACKET_SIZE      3
#endif

#ifdef      CONFIG_USB_PACKET_128
#define     CONFIG_PACKET_SIZE      4
#endif

#ifdef      CONFIG_USB_PACKET_256
#define     CONFIG_PACKET_SIZE      5
#endif

#ifdef      CONFIG_USB_PACKET_512   
#define     CONFIG_PACKET_SIZE      6
#endif


#define BULK_PACKET_SIZE		    (1 << (CONFIG_PACKET_SIZE + 3)) // ##

#define EP0							0

#define BULK_EP_NUM             1   //###

//#define CONFIG_DBUFFER          1   //##

#define BULKTX_EP_FIFO_ADDRESS		(EP0_PACKET_SIZE * 2)

#if     CONFIG_DBUFFER
#define BULKRX_EP_FIFO_ADDRESS		(BULKTX_EP_FIFO_ADDRESS + BULK_PACKET_SIZE * 2)
#else
#define BULKRX_EP_FIFO_ADDRESS		(BULKTX_EP_FIFO_ADDRESS + BULK_PACKET_SIZE)
#endif


#define USB_BULK_TX				1
#define USB_BULK_RX				2


// Endpoint interrupt status bits
#define USB_INTR_CLR				0x00		// From USB function controller specification
#define USB_INTR_OUT_RCV		0x01		// OUTtokenRcv bit
#define USB_INTR_IN_RCV			0x02		// INtokenRcv bit
#define USB_INTR_OUT_NAK		0x04		// OUTtranNAK bit
#define USB_INTR_IN_NAK			0x08		// INtranNAK bit
#define USB_INTR_OUT_ERR		0x10		// OUTtranErr bit
#define USB_INTR_IN_ERR			0x20		// INtranErr bit
#define USB_INTR_OUT_ACK		0x40		// OUTtranACK bit
#define USB_INTR_IN_ACK			0x80		// INtranACK bit

#define USB_BUFFER_SIZE			128		// USB buffer size


#define MSB(x)						(((x) >> 8) & 0xFF)
#define LSB(x)						((x) & 0xFF)

/* control endpoint status */
#define USB_IDLE					0
#define USB_TRANSMIT				1
#define USB_RECEIVE				2

#define USB_MS_NULL				0
#define USB_MS_READ				1
#define USB_MS_WRITE				2

#define USB_BULK_IN				0
#define USB_BULK_OUT				1

#define MAX_ENDPOINTS			(unsigned char)0x7


#define USB_PROTOCOL_EDICT		2	

/* protocol status */
#define USBFSM_IDLE           0
#define USBFSM_DATAIN         1
#define USBFSM_DATAOUT        2
#define USBFSM_CBWPROC        3
#define USBFSM_CSW            4
#define USBFSM_ABORT          5
#define USBFSM_CSWPROC        6
#define USBFSM_STALL          7
#define USBFSM_HANDSHAKE		8



/* protocol error code */
#define USBERRCODE_SUCCESS		(0)
#define USBERRCODE_USBERROR	(-1)	// USB连接异常
#define USBERRCODE_BUSERROR	(-2)	// DMA总线异常
#define USBERRCODE_CMDERROR	(-3)	// 命令错误
#define USBERRCODE_CASECBW		(-4)	// MS CBW 错误
#define USBERRCODE_SPCERROR	(-5)










/****************************************************************************
*
*	USB Configure
*
****************************************************************************/

// GOT FROM MENUCONFIG

#define CONFIG_USB_HIGH_SPEED   CONFIG_USB_HS  /* only FIB chip can work on High Speed*/





/* USB DMA OPERATION STATUS */

#define DMA_USB_RX      0
#define DMA_USB_TX_SINGLE      1
#define DMA_USB_IDLE    2
#define DMA_USB_WAITEMPTY   3
#define DMA_USB_SINGLERX    4
#define DMA_USB_TX_MULTIPLE 5



typedef unsigned char			u8_t;
typedef unsigned short			u16_t;
typedef unsigned int			u32_t;



typedef volatile unsigned char  vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned long  vu32;

#define	USB_NC_ICOLL_CLEAR	(1 << (57-32))
// INTRTXE (Address: 06h)
#define	INTRTXE_EP4_Tx				 4
#define	INTRTXE_EP3_Tx				 3
#define	INTRTXE_EP2_Tx				 2
#define	INTRTXE_EP1_Tx				 1
#define	INTRTXE_EP0					 0

// INTRRXE (Address: 08h)
#define	INTRRXE_EP4_Rx				4
#define	INTRRXE_EP3_Rx				3
#define	INTRRXE_EP2_Rx				2
#define	INTRRXE_EP1_Rx				1

// POWER (Address: 01h)
#define POWERUSB_Resume             2

// INTRUSB (Address: 0Ah)
#define	INTRUSB_VBusError			7
#define	INTRUSB_SessReq			    6
#define	INTRUSB_Discon				5
#define	INTRUSB_Conn				4
#define	INTRUSB_SOF					3
#define	INTRUSB_Reset				2
#define	INTRUSB_Resume				1
#define	INTRUSB_Suspend			    0

// CSR0 Bit Field Descriptions
#define	CSR0_FlushFIFO				8
#define	CSR0_ServicedSetupEnd	    7
#define	CSR0_ServicedRxPktRdy	    6
#define	CSR0_SendStall				5
#define	CSR0_SetupEnd				4
#define	CSR0_DataEnd				3
#define	CSR0_SentStall				2
#define	CSR0_TxPktRdy				1
#define	CSR0_RxPktRdy				0

// TXCSR Bit Field Descriptions
#define	TXCSR_AutoSet				15
#define	TXCSR_ISO					14
#define	TXCSR_Mode					13
#define	TXCSR_Mode_TX				13
#define	TXCSR_Mode_RX				  
#define	TXCSR_DMAReqEnab			12
#define	TXCSR_FrcDataTog			11
#define	TXCSR_DMAReqMode			10
#define	TXCSR_IncompTx				7 
#define	TXCSR_ClrDataTog			6 
#define	TXCSR_SentStall			    5
#define	TXCSR_SendStall			    4 
#define	TXCSR_FlushFIFO			    3 
#define	TXCSR_UnderRun			    2
#define	TXCSR_FIFONotEmpty		    1
#define	TXCSR_TxPktRdy				0

// RXCSR Bit Field Descriptions
#define	RXCSR_AutoClear			    15
#define	RXCSR_ISO					14
#define	RXCSR_DMAReqEnab			13
#define	RXCSR_DisNyet				12
#define	RXCSR_DMAReqMode			11
#define	RXCSR_IncompRx				8 
#define	RXCSR_ClrDataTog			7 
#define	RXCSR_SentStall			    6
#define	RXCSR_SendStall			    5
#define	RXCSR_FlushFIFO			    4
#define	RXCSR_DataError			    3
#define	RXCSR_OverRun				2
#define	RXCSR_FIFOFull				1
#define	RXCSR_RxPktRdy				0

// devctrlUSB (Address: 60h)
#define DEVCTRLUSB_BDevice          7
#define DEVCTRLUSB_HostMode         2
#define DEVCTRLUSB_Session          0

// usb dma reg
//                                            
#define CNTL_BurstMode0             0
#define CNTL_BurstMode1             1
#define CNTL_BurstMode2             2
#define CNTL_BurstMode3             3
#define CNTL_BurstMode              9



#define CNTL_BusError               8 
#define CNTL_EpNum                  4
#define CNTL_INTRE                  3 

#define CNTL_DMAMode0               0       
#define CNTL_DMAMode1               1 
#define CNTL_DMAMode                2  

#define CNTL_DICWriteRx             0       
#define CNTL_DICReadTx              1  
#define CNTL_DIC                    1

#define CNTL_DMAEN                  0 


// usb related registrators map

// usb phy register


#define USB_PWD        	0x8007C000
#define USB_TX       	0x8007C010
#define USB_RX          0x8007C020
#define USB_CTRL       	0x8007C030
#define USB_SYSCTRL     0x8007C090
#define USB_ANALOG      0x8007C0A0

// usb clk in clkctrl register
#define UTMI_CLK        0x80040070
#define USB_CLK         0x8001C000
#define CPU_CLK_REG     0x80040020
#define CLK_CTRL        0x80040000

//irq register

#define USB_DMA_IRQ     11
#define USB_MC_IRQ      57
#define USB_NEGEDGE_IRQ 59
#define USB_POSEDGE_IRQ 60
#define PRIORITY_BASE   0x80000060
#define ICOLL_CLEAR1    0x800001E0


#define HW_USBPHY_SYSCTRL (0x8007c000 + 0x90)		//APB PHY register address

//usb mentor register

#define USB_BaseAddr    0x90000000	//USB base address

#define  R_USB_FAddr        (vu8*) (0x00 + USB_BaseAddr)                          
#define  R_USB_Power        (vu8*) (0x01 + USB_BaseAddr)               
#define  R_USB_IntrTx       (vu16*)(0x02 + USB_BaseAddr)               
#define  R_USB_IntrRx       (vu16*)(0x04 + USB_BaseAddr)               
#define  R_USB_IntrTxE      (vu16*)(0x06 + USB_BaseAddr)               
#define  R_USB_IntrRxE      (vu16*)(0x08 + USB_BaseAddr)               
#define  R_USB_IntrUSB      (vu8*) (0x0A + USB_BaseAddr)               
#define  R_USB_IntrUSBE     (vu8*) (0x0B + USB_BaseAddr)               
#define  R_USB_Frame        (vu16*)(0x0C + USB_BaseAddr)               
#define  R_USB_Index        (vu8*) (0x0E + USB_BaseAddr)               
#define  R_USB_Testmode     (vu8*) (0x0F + USB_BaseAddr)                                                        
#define  R_USB_TxMaxP       (vu16*)(0x10 + USB_BaseAddr)                  
#define  R_USB_TxCSR        (vu16*)(0x12 + USB_BaseAddr)                  
#define  R_USB_RxMaxP       (vu16*)(0x14 + USB_BaseAddr)                  
#define  R_USB_RxCSR        (vu16*)(0x16 + USB_BaseAddr)                  
#define  R_USB_RxCount      (vu16*)(0x18 + USB_BaseAddr)                  
#define  R_USB_FIFO0        (vu8*) (0x20 + USB_BaseAddr)                                                       
#define  R_USB_DevCtl       (vu8*) (0x60 + USB_BaseAddr)                    
#define  R_USB_TxFIFOsz     (vu8*) (0x62 + USB_BaseAddr)                    
#define  R_USB_RxFIFOsz     (vu8*) (0x63 + USB_BaseAddr)                    
#define  R_USB_TxFIFOadd    (vu16*)(0x64 + USB_BaseAddr)                   
#define  R_USB_RxFIFOadd    (vu16*)(0x66 + USB_BaseAddr)                                
#define  R_USB_DmaIntr      (vu16*)(0x200 + USB_BaseAddr) 
#define  R_USB_DmaCtrl      (vu32*)(0x204 + USB_BaseAddr) 
#define  R_USB_DmaAddr      (vu32*)(0x208 + USB_BaseAddr) 
#define  R_USB_DmaCount     (vu32*)(0x20c + USB_BaseAddr) 



// bit operations
#define check_u8reg_assert(value,i)  ( (value & (1<<i)) !=0 )
#define check_u16reg_assert(value,i) ( (value & (1<<i)) !=0 )

#define check_u8reg_deassert(value,i)  ( (value & (1<<i)) ==0 )
#define check_u16reg_deassert(value,i) ( (value & (1<<i)) ==0 )

//#define get_Rx_fifo_count(usb_ctrl)	((usb_ctrl)->R_RxCount & 0x1FFF)


#define set_charreg(reg,i)          do { *reg|=(1<<i); } while(0)

#define set_shortreg(reg,i)         do { *reg|=(1<<i); } while(0)
#define set_intreg(reg,i)           do { *reg|=(1<<i); } while(0)
#define clr_charreg(reg,i)          do { *reg&=(~(1<<i)); } while(0)  
#define clr_shortreg(reg,i)         do { *reg&=(~(1<<i)); } while(0)  
#define clr_intreg(reg,i)           do { *reg&=(~(1<<i)); } while(0)  

#define setindex(ep_num)   outx(ep_num,R_USB_Index)




/******************************************************************************/
/*		structure and union definitions														*/
/******************************************************************************/
typedef struct _usb_flags
{
	unsigned char bus_reset;
	unsigned char resume;
	unsigned char suspend;
	unsigned char NonJ;
	unsigned char setup_packet;
	unsigned char in_isr;
	unsigned char control_state;
	unsigned char configuration;
	unsigned char command_state;
	unsigned char protocol;
	unsigned long dev;
} USBFLAGS;



typedef struct _control_xfer
{
	DEVICE_REQUEST	DeviceRequest;
	u32_t				dwLength;
	u32_t				dwCount;
	u32_t				dwPacket;
	unsigned char	*pData;
	unsigned char  DataBuffer[512];
} __attribute__((packed)) CONTROL_XFER;

extern USBFLAGS		UsbFlags;
extern CONTROL_XFER	ControlData;

unsigned char   usb_dma_status;

struct device * usb_mem_dev;   
//struct devmem * usb_mem_dev_i; 














































#ifdef BIOS_TEST

#	define CANCEL       0	/* general req to force a task to cancel */
#	define HARD_INT     2	/* fcn code for all hardware interrupts */
#	define DEV_READ	  3	/* fcn code for reading from device */
#	define DEV_WRITE    4	/* fcn code for writing to device */
#	define DEV_IOCTL    5	/* fcn code for ioctl */
#	define DEV_OPEN     6	/* fcn code for opening device */
#	define DEV_CLOSE    7	/* fcn code for closing device */
#	define DEV_SCATTER  8	/* fcn code for writing from a vector */
#  define DEV_GATHER   9	/* fcn code for reading into a vector */
#  define DEV_FORMAT	  10	/* fcn code for format device */	
#	define OPTIONAL_IO  16	/* modifier to DEV_* codes within vector */

#define USBTASK				-10	/* task to process usb communication protocol */

struct partition {
	u32_t		base;			/* byte offset to the partition start */
	u32_t		size;			/* number of bytes in the partition */
	u32_t		cylinders;	/* disk geometry */
	u32_t		heads;
	u32_t		sectors;
};


#define _IOCPARM_MASK	0x1FFF
#define _IOC_VOID			0x20000000
#define _IOCTYPE_MASK	0xFFFF
#define _IOC_IN			0x40000000
#define _IOC_OUT			0x80000000
#define _IOC_INOUT		(_IOC_IN | _IOC_OUT)

#define _IO(x,y)			((x << 8) | y | _IOC_VOID)
#define _IOR(x,y,t)		((x << 8) | y | ((sizeof(t) & _IOCPARM_MASK) << 16) |	_IOC_OUT)
#define _IOW(x,y,t)		((x << 8) | y | ((sizeof(t) & _IOCPARM_MASK) << 16) |	_IOC_IN)
#define _IORW(x,y,t)		((x << 8) | y | ((sizeof(t) & _IOCPARM_MASK) << 16) |	_IOC_INOUT)


#define FIOREAD			0
#define FIOWRITE			1

#define DIOCGETP			_IOR('d', 7,  struct partition)

/* Notification Event Type 通知事件类型定义 */
#define	NTFTYPE_INTR			0			// interrupt notification type 中断通知事件类型
#define	NTFTYPE_TASK			1			// task notification type 任务通知事件类型

typedef	void *					NTFEVENT;
typedef	unsigned int			NTFTYPE;

#define	NTFEVENT_USB_TIMEEXPIRED	0
#define	NTFEVENT_USB_LINKACTIVE		1	
#define	NTFEVENT_USB_LOCKED			2	
#define	NTFEVENT_USB_UNLOCK			3



#define USB_PROTOCOL_FML	0
#define USB_PROTOCOL_MS		1   // #
void		UsbEventNotify		(NTFEVENT ntfEvent, NTFTYPE ntfType);

#endif


int				usb_EPc_read				(unsigned char *buf, unsigned char len);
int				usb_EP0_transmit			(unsigned char *buff, u32_t len);
int				usb_EPb_transmit			(unsigned char *buff, u32_t len);
void				usb_EP0_setup				(void);
void				usb_config					(void);
void				usb_unconfig				(void);
void				usb_bcu_init				(void);
void				usb_dma_init				(void);
void				usb_Active					(void);
void				usb_Deactive				(void);
void				usb_reset					(void);
int                 usb_init(struct device * dev);
void				usb_StallEP0				(void);
void				usb_StallBulkEP			(void);
void				usb_EPx_reset				(int ep);
void				usb_EPx_clear_fifo		(int ep);
void				usb_EPx_set_stall			(int ep);
void				usb_EPx_clr_stall			(int ep);
void				usb_EPx_clr_toggle		(int ep);

void 				printHex (u8_t *hex, int len);
unsigned long   usb_bulk_loadfifo(unsigned char *buf,unsigned long le,unsigned short ep_num);
unsigned long   usb_bulk_unloadfifo(unsigned char *buf,unsigned long le,unsigned short ep_num);
void            ep_bulk_tx(void);
void            ep_bulk_rx(void);
int             service_transmit(void);
int             service_receive();
int             service_transmit_buffer();
int             do_irq_usb_dma(int irq);
int             do_irq_usb(int priv);
int             usb_ioctl(struct device * dev,unsigned int cmd,unsigned long arg);
int             usb_probe(struct device * dev);
void            hs_calibration(void);
void            otg_init(void);
void            usb_interrupt_enable(void);
int             usb_EPx_read (unsigned char *buf, unsigned char len, int ep);
void            edict_control_handler(void);
void            get_string_descriptor(void);
void            ms_usb_config (void);
void            ms_usb_unconfig (void);

#endif
