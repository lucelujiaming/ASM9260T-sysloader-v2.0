
//****************************************************************************
//
//	Copyright (C) 2004-2005 Zhuo YongHong
//
//	Author	ZhuoYongHong
//
//	File name: bulkonly.h
//	  This is definition of transport control protocol for bulk-only transfer
//
//	Revision history
//
//		2005.01.15	ZhuoYongHong add definition about Bulk-Only Transport 
//
//****************************************************************************
#ifndef __BULK_ONLY_H__
#define __BULK_ONLY_H__

#define CSW_GOOD				0x00
#define CSW_FAIL				0x01
#define CSW_PHASE_ERROR		0x02

#define MS_CBW_SIZE			0x1F
#define MS_CBW_SIGNATURE	0x43425355l
#define MS_CSW_SIGNATURE	0x53425355l

#define CBW_FLAG_IN			0x80
#define MAX_CDBLEN			0x10

#define RAM_DISK_SIZE   0xb00000
#define BLOCK_SIZE      512
//#define SRAM_ADD        0x40000000  // USE sram, so don't need d-cache
#define SRAM_BUF_SIZE   2048


#define SINGLEPACKET    0
#define MULTIPLEPACKET  1

// for TPBulk_CheckDataTransfer
#define	USB_IN	0x80
#define	USB_OUT	0x00

#define WITHOUT_FDU

// Normal FSM for Set Command
//  IDLE -> SETUP[IRQL-1] -> DataOut[IRQL_1] -> RequestProc[IRQL_0] -> Status Stage
// Normal FSM for Get Command
//  IDLE -> SETUP[IRQL-1] -> RequestProc[IRQL_0] -> DataIn[IRQL-1]-> Status Stage

#define	TPBulk_SetBOTState(state)	UsbFlags.command_state = state




/* SPC-3 commands */
#define SPC_CMD_INQUIRY							0x12	// Tells the host the drive information.
#define SPC_CMD_READCAPACITY					0x25	// Tells the host the media sector information.
#define SPC_CMD_READ10							0x28	// Reads the specified sector volume data from a specified sector.
#define SPC_CMD_WRITE10							0x2A	// Writes the specified sector volume data to a specified sector.
#define SPC_CMD_REQUESTSENSE					0x03	// If an error occurred for the previous command, this
#define SPC_CMD_READFORMAT_CAPACITY             0x23
																//		tells the host what kind of error occurred.
#define SPC_CMD_MODESENSE10					0x5A	// Tells the host the drive status.
#define SPC_CMD_PRVENTALLOWMEDIUMREMOVAL	0x1E	// Inhibits/enables installing and removing media.
#define SPC_CMD_TESTUNITREADY					0x00	// Checks whether or not a medium can be used.
#define SPC_CMD_VERIFY10						0x2F	// Confirms whether or not the data in a medium can be accessed.


#define SCC2_DEVICE								0x0C    /* Storage Controller device */
#define SES_DEVICE								0x0D    /* Enclousre Service device */
#define SPC_DEVICE								0x0E    /* SPC device */
#define OCRW_DEVICE								0x0F    /* optical Card Reader /writer Device */

#define VPDPAGE_SERIAL_NUMBER					0x80
#define VPDPAGE_DEVICE_IDENTITY				0x83

/* Mode Sense/Select page constants. */
#define MODE_PAGE_SPC_DEVICE_PARAMETERS	0x06


//	SCSI-2 additional sense codes
#define ASC_INVALID_FIELD_IN_CDB				0x24

#define BUFFER_SIZE     1024
#define TRANS_OVER          0
#define TRANS_ING           1
#define TRANS_EMPTY_PACK    2
#define TRANS_BSW_OVER      3
#define ALL_TRANS_OVER      4

// Format descriptor code definition 
#define UNFORMATTED     1
#define FORMATED        2
#define NOCARTRIDGE     3


 

enum _SENSE_DATA {
	SD_NOSENSE = 0,
	SD_RECOVEREDERROR,
	SD_NOTREADY,
	SD_MEDIUMERROR,
	SD_HWERROR,
	SD_ILLEGALREQUEST,
	SD_UNITATTENTION,
	SD_DATAPROTECT,
	SD_BLANKCHECK,
	SD_VENDORSPECIFIC,
	SD_COPYABORTED,
	SD_ABORTEDCOMMAND,
	SD_OBSOLETE,
	SD_VOLUMEOVERFLOW,
	SD_MISCOMPARE,
	SD_RESERVED,
};

/*****************************************/
/*Structure for bulk_only data transmit***/
/*****************************************/
typedef struct _bulk_only_in_server
{
    unsigned char*  buffer_addr;
    unsigned long   offset_now;
    unsigned long   data_already_trans;
    unsigned long   data_left_len;
    unsigned long   dcswtag;
    unsigned char   trans_status;
    unsigned long   data_last_trans;
}__attribute__((packed)) bulk_only_in_server;


/*****************************************/
/*Structure for bulk_only data recieve***/
/*****************************************/
typedef struct _bulk_only_out_server
{
    unsigned char*  buffer_addr;
    unsigned long   offset_now;
    unsigned long   data_already_got;
    unsigned long   data_left_len;
    unsigned char   trans_status;
    unsigned long   data_last_got;
}__attribute__((packed)) bulk_only_out_server;

/***********************************************************************************/
/* Command Descriptor Block                                                        */
/*      _SPC : SPC-3 SPC primary Command - 3                                       */
/***********************************************************************************/

/* Generic */
typedef struct _GENERIC_SPC 
{
	unsigned char OperationCode;
	unsigned char Reserved[8];
	unsigned char Control;
} __attribute__((packed)) GENERIC_SPC, *PGENERIC_SPC;

/* Read Command */
typedef struct _READ_SPC 
{
	unsigned char OperationCode;	/* 10H */
	unsigned char VendorSpecific;
	unsigned char LBA_3;			
	unsigned char LBA_2;
	unsigned char LBA_1;
	unsigned char LBA_0;
	unsigned char Reserved;
	unsigned char XferLength_1;		
	unsigned char XferLength_0;
	unsigned char Control;
} __attribute__((packed)) READ_SPC, *PREAD_SPC;

/* Read Capacity command */
typedef struct _READ_CAPACITY_SPC 
{
	unsigned char OperationCode;	/* 25H */
	unsigned char LogicalUnitNumber	: 3;
	unsigned char Reserved0				: 4;
	unsigned char RelAdr					: 1;
	unsigned char LBA_3;
	unsigned char LBA_2;
	unsigned char LBA_1;
	unsigned char LBA_0;
	unsigned char Reserved1;
	unsigned char Reserved2;
	unsigned char Reserved3				: 7;
	unsigned char PMI						: 1;
	unsigned char Control;
} __attribute__((packed)) READ_CAPACITY_SPC, *PREAD_CAPACITY_SPC;


/* Write10 Command */
typedef struct _WRITE_SPC 
{
	unsigned char OperationCode;	/* 2AH */
	unsigned char Reserved0				: 3;
	unsigned char FUA						: 1;	
	unsigned char Reserved1				: 4;
	unsigned char LBA_3;			
	unsigned char LBA_2;
	unsigned char LBA_1;
	unsigned char LBA_0;
	unsigned char Reserved2;
	unsigned char XferLength_1;		
	unsigned char XferLength_0;
	unsigned char Control;
} __attribute__((packed)) WRITE_SPC, *PWRITE_SPC;

/* VERIFY Command */
typedef struct _VERIFY_SPC 
{
	unsigned char OperationCode;	/* 2FH */
	unsigned char Reserved0;
	unsigned char LBA_3;				/* Big Endian */
	unsigned char LBA_2;
	unsigned char LBA_1;
	unsigned char LBA_0;
	unsigned char Reserved1;
	unsigned char VerifyLength_1;	/* Big Endian */
	unsigned char VerifyLength_0;
	unsigned char Control;
} __attribute__((packed)) VERIFY_SPC, *PVERIFY_SPC;

/* READ FORMAT CAPACITY COMMAND */
typedef struct _READ_FORMAT
{
    unsigned char OperationCode;    /* 23h */
    unsigned char LogicalUnitNumber;
    unsigned long reserved0;
    unsigned char reserved1;
    unsigned char ALL_LEN_1;
    unsigned char ALL_LEN_0;

} __attribute__((packed)) READ_FORMAT, *PREADFORMAT_SPC ;

typedef struct _CapacityListHeader
{
    unsigned char reserved0;
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char CapacityListLen;
} __attribute__((packed)) CapacityListHeader, *PCapacityListHeader_SPC;

/* here for current ,maximum and formattable capacity descriptors */
typedef struct _Format_CapacityDesc
{
    unsigned char NumOfBlocks_3;
    unsigned char NumOfBlocks_2;
    unsigned char NumOfBlocks_1;
    unsigned char NumOfBlocks_0;
    unsigned char Desc_Code;
    unsigned char BlockLen_2;
    unsigned char BlockLen_1;
    unsigned char BlockLen_0;

} __attribute__((packed)) Format_CapacityDesc, *PFormat_CapacityDesc_SPC;


/* INQUIRY Command */
typedef struct _INQUIRY_SPC {
	unsigned char OperationCode;	/* 12H */
	unsigned char EVPD					:1 ;
	unsigned char CMDDT					:1 ;
	unsigned char Reserved0				:6 ;
	unsigned char PageCode;
	unsigned char Reserved1;
	unsigned char AllocationLength;
	unsigned char Control;
} __attribute__((packed)) INQUIRY_SPC, *PINQUIRY_SPC;

typedef struct _INQUIRY_DATA {
	unsigned char DeviceType			: 5;		// should be 0x0E
	unsigned char Qualifier				: 3;		
	unsigned char Reserved1				: 7;
	unsigned char RMB						: 1;	
	unsigned char Version;
	unsigned char DataFormat			: 4;
	unsigned char HiSup					: 1;
	unsigned char NACA					: 1;
	unsigned char Reserved2				: 1;
	unsigned char AERC					: 1;
	unsigned char AdditionalLengh;
	unsigned char Reserved3				: 7;
	unsigned char SCCS					: 1;
	unsigned char Reserved4;
	unsigned char Reserved5				: 3;
	unsigned char Linked					: 1;		// fixed to zero
	unsigned char Reserved6				: 3;
	unsigned char Reladr					: 1;		// fixed to zero
	unsigned char VendorID[8];
	unsigned char ProductID[16];
	unsigned char ProductRevision[4];
	unsigned char VendorSpecific[20];
	unsigned char Reserved7;
	unsigned char Reserved8;
	unsigned short VersionDescriptor[8];
	unsigned char Reserved9[22];
} __attribute__((packed)) INQUIRY_DATA, *PINQUIRY_DATA;

typedef struct _SUPPORT_VITAL_PAGE {
	unsigned char DeviceType			: 5;
	unsigned char Qualifier				: 3;
	unsigned char PageCode;
	unsigned char Reserved;
	unsigned char PageLength;
	unsigned char PageList[3];		// now support 0, 0x80, 0x83
} __attribute__((packed)) SUPPORT_VITAL_PAGE, *PSUPPORT_VITAL_PAGE;

/* Mode Sense */
typedef struct _MODE_SENSE_SPC {
	unsigned char OperationCode;	/* 1AH */
	unsigned char Reserved0              : 3 ;
	unsigned char DisableBlockDescriptor : 1 ;
	unsigned char Reserved1              : 4 ;
	unsigned char PageCode               : 6 ;
	unsigned char PageControl            : 2 ;
	unsigned char SubPageCode;
	unsigned char ParameterLen;
	unsigned char Control;
} __attribute__((packed)) MODE_SENSE_SPC, * PMODE_SENSE_SPC;

typedef struct _MODE_PARAMETER_HEADER6 {
	unsigned char ModeDataLength;
	unsigned char MediumType;
	unsigned char DeviceSpecificParameter;
	unsigned char BlockDescriptorLength;
} __attribute__((packed)) MODE_PARAMETER_HEADER6, *PMODE_PARAMETER_HEADER6;

/* See SCSI Primary Commands - 3 (SPC-3) */
/* Informational Exceptions Control mode page */
typedef struct _IEC_MODE_PAGE {
	unsigned char PageCode					: 6;
	unsigned char SFB							: 1;
	unsigned char PS							: 1;
	unsigned char PageLength;
	unsigned char LogErr						: 1;
	unsigned char Reserved0					: 1;
	unsigned char Test						: 1;
	unsigned char DExcpt						: 1;
	unsigned char EWasc						: 1;
	unsigned char EBF							: 1;
	unsigned char Reserved1					: 1;
	unsigned char PERF						: 1;
	unsigned char MRIE						: 4;
	unsigned char Reserved2					: 4;
	unsigned int  IntervalTimer;
	unsigned int  ReportCount;
} __attribute__((packed)) IEC_MODE_PAGE, *PIEC_MODE_PAGE;

/* See SCSI Block Commands - 2 (SBC-2) */
/* Read-Write Error Recovery mode page */
typedef struct _RWER_MODE_PAGE {
	unsigned char PageCode					: 6;		// 0x00	0x01 fixed
	unsigned char Reserved0					: 1;
	unsigned char PS							: 1;
	unsigned char PageLength;							// 0x01	x0A fixed
	unsigned char DCR							: 1;		// 0x02
	unsigned char DTE							: 1;
	unsigned char PER							: 1;
	unsigned char EER							: 1;
	unsigned char RC							: 1;
	unsigned char TB							: 1;
	unsigned char ARRE						: 1;
	unsigned char AWRE						: 1;
	unsigned char ReadRetryCount;						// 0x03
	unsigned char Obsolete[3];							// 0x04	
	unsigned char Reserved1;							// 0x07
	unsigned char WriteRetryCount;					// 0x08
	unsigned char Reserved2;							// 0x09
	unsigned short RecoveryTimeLimit;				// 0x0A
} __attribute__((packed)) RWER_MODE_PAGE, *PRWER_MODE_PAGE;

/* prevent/allow medium removal */
typedef struct _MEDIA_REMOVAL_SPC {
	unsigned char OperationCode;    /* 1EH */
	unsigned char Reserved0[3];
	unsigned char Prevent				: 2 ;
	unsigned char Reserved1				: 6 ;
	unsigned char Control;
} __attribute__((packed)) MEDIA_REMOVAL_SPC, *PMEDIA_REMOVAL_SPC;


/* Request Sense*/
typedef struct _REQUEST_SENSE_SPC {
	unsigned char OperationCode;    /* 03H */
	unsigned char Reserved[3];
	unsigned char AllocationLen;
	unsigned char Control;
} __attribute__((packed)) REQUEST_SENSE_SPC, *PREQUEST_SENSE_SPC;



typedef struct _REQUEST_SENSE_DATA 
{
	unsigned char ResponseCode          : 7;		// 0x00
	unsigned char Valid                 : 1;
	unsigned char SegmentNum;							// 0x01
	unsigned char SenseKey              : 4;		// 0x02
	unsigned char Reserved0             : 1;
	unsigned char WrongLenIndicator     : 1;
	unsigned char EndofMedium           : 1;
	unsigned char FileMark              : 1;
	unsigned char Info_3;								// 0x03
	unsigned char Info_2;								// 0x04
	unsigned char Info_1;								// 0x05
	unsigned char Info_0;								// 0x06
	unsigned char AdditionalSenseLen;				// 0x07
	unsigned char CommandSpecInfo_3;					// 0x08
	unsigned char CommandSpecInfo_2;					// 0x09
	unsigned char CommandSpecInfo_1;					// 0x0A
	unsigned char CommandSpecInfo_0;					// 0x0B
	unsigned char ASC;									// 0x0C
	unsigned char ASCQ;									// 0x0D		
	unsigned char FieldReplacableUnitCode;			// 0x0E
	
	unsigned char SenseKeySpec_0        : 7;		// 0x0F
	unsigned char SKSV						: 1;		// 
	unsigned char SenseKeySpec_1;						// 0x01
	unsigned char SenseKeySpec_2;						// 0x11
	// addition sense bytes
} __attribute__((packed)) REQUEST_SENSE_DATA, *PREQUEST_SENSE_DATA;


/* Test Unit Ready*/
typedef struct _TEST_UNIT_SPC {
	unsigned char OperationCode;    /* 00H */
	unsigned char Reserved[4];
	unsigned char Control;
} __attribute__((packed)) TEST_UNIT_SPC, *PTEST_UNIT_SPC;


/* READ CAPACITY data */
typedef struct _READ_CAPACITY_DATA {
	unsigned char LogicalBlockAddress_3;
	unsigned char LogicalBlockAddress_2;
	unsigned char LogicalBlockAddress_1;
	unsigned char LogicalBlockAddress_0;
	unsigned char BlockLength_3;
	unsigned char BlockLength_2;
	unsigned char BlockLength_1;
	unsigned char BlockLength_0;
} __attribute__((packed)) READ_CAPACITY_DATA, *PREAD_CAPACITY_DATA;

typedef union _CDB_SPC {
	/* SPC commands */
	GENERIC_SPC             SpcCdb_Generic;
	READ_SPC                SpcCdb_Read;
	READ_CAPACITY_SPC       SpcCdb_ReadCapacity;          
	VERIFY_SPC              SpcCdb_Verify;
	WRITE_SPC               SpcCdb_Write;
	INQUIRY_SPC             SpcCdb_Inquiry;
	MODE_SENSE_SPC          SpcCdb_ModeSense;
	MEDIA_REMOVAL_SPC       SpcCdb_Remove;
	REQUEST_SENSE_SPC       SpcCdb_RequestSense;
	TEST_UNIT_SPC           SpcCdb_TestUnit;
} __attribute__((packed)) CDB_SPC, *PCDB_SPC;

unsigned char SPC_Handler			(CDB_SPC *cdb, u8_t dir, u32_t transer_length);
unsigned char SPC_Read				(CDB_SPC *cdb);
unsigned char SPC_ReadCapacity	(CDB_SPC *cdb);
unsigned char SPC_Verify			(CDB_SPC *cdb);
unsigned char SPC_ReadFormat    (CDB_SPC *cdb);
unsigned char SPC_Write				(CDB_SPC *cdb);
unsigned char SPC_Inquiry			(CDB_SPC *cdb);
unsigned char SPC_ModeSense		(CDB_SPC *cdb);
unsigned char SPC_LockMedia		(CDB_SPC *cdb);
unsigned char SPC_TestUnit			(CDB_SPC *cdb);
unsigned char SPC_RequestSense	(CDB_SPC *cdb);

//static void			  SPC_BuildSenseData	(unsigned char Type, unsigned char ASC, unsigned char ASCQ);



//typedef struct _BULK_XFER
//{
//	unsigned char   Abort;      /*Indicate this tranaction shall be aborted ASAP*/
//	unsigned long   dLength;
//	unsigned char   dCount;
//	ADDRESS         Addr;
//} BULK_XFER, *PBULK_XFER;

typedef struct _COMMAND_BLOCK_WRAPPER {
    unsigned long   dCBW_Signature;
    unsigned long   dCBW_Tag;
    unsigned long   dCBW_DataXferLen;
    unsigned char   bCBW_Flag;
    unsigned char   bCBW_LUN;
    unsigned char   bCBW_CDBLen;
    unsigned char   CBW_CDB[MAX_CDBLEN];    // MAX_CDBLEN = 0x10;
} __attribute__((packed)) CBW, *PCBW;

typedef struct _COMMAND_STATUS_WRAPPER {
    unsigned long   dCSW_Signature;
    unsigned long   dCSW_Tag;
    unsigned long   dCSW_DataResidue;
    unsigned char   bCSW_Status;
} __attribute__((packed)) CSW, *PCSW;



/**************************************************************************/
/*  USB Class Request Functions                                           */
/*  and                                                                   */
/*  Public Functions                                                      */
/**************************************************************************/


/* Host Device Disagreement Matrix*/
enum _HOST_DEV_DISAGREE {
	CASEOK = 0,
	CASE1,
	CASE2,
	CASE3,
	CASE4,
	CASE5,
	CASE6,
	CASE7,
	CASE8,
	CASE9,
	CASE10,
	CASE11,
	CASE12,
	CASE13,
	CASECBW,
	CASEFAIL,
};

//##############################################################
// global var

extern CBW				TPBulk_CommandBlock;
extern CSW				TPBulk_CommandStatus;

extern u8*  mass_buf;

extern u32   lba_offset;
extern u32   buf_offset;
extern u32  buffer_size;

bulk_only_in_server     bulk_only_in_now;
bulk_only_out_server    bulk_only_out_now; 
//REQUEST_SENSE_DATA request_sense_data;
static const INQUIRY_DATA StandardInquiryData;

/*************************************************************************
 C[ommand]D[ata]S[tatus] architecture for mass storage device over Bulk
 only Transport
*************************************************************************/
#define	CONTROL_PACKET_SIZE			EP0_PACKET_SIZE
#define	BULKIN_PACKET_SIZE			EPa_PACKET_SIZE
#define	BULKOUT_PACKET_SIZE			EPd_PACKET_SIZE
#define USB_CLASS_REQUEST_USBSTOR_RESET         0xFF
#define USB_CLASS_REQUEST_USBSTOR_GETMAXLUN     0xFE

#define	TPBulk_Read						usb_EPc_read
#define	TPBulk_Write					usb_EPb_write
#define	TPBulk_Transmit				usb_EPb_transmit



void		TPBulk_ClassRequestHandler				(void);
int		TPBulk_CBWHandler							(void);
void		TPBulk_ErrorHandler						(unsigned char HostDevCase, u32_t DevXferCount);
int		TPBulk_CSWHandler							(void);

void		TPBulk_BusReset_IRQL1					(void);
void		TPBulk_BusReset_IRQL0					(void);

void		TPBulk_SuspendChange_IRQL1				(void);
void		TPBulk_SuspendChange_IRQL0				(void);

unsigned char		TPBulk_GetMaxLUN							(void);

/**************************************************************************/
/* Bulk Only Transport support functions                                  */
/**************************************************************************/
void		TPBulk_SoftReset							(void);
void		TPBulk_HardReset							(void);

void		TPBulk_StallEP0							(void);
void		TPBulk_SingleTransmitEP0				(unsigned char * buf, unsigned char len);

void		TPBulk_StallBulkInEP						(void);
void		TPBulk_StallBulkOutEP					(void);
void		TPBulk_ClearBulkInEP						(u8 ep_num);
void		TPBulk_ClearBulkOutEP					(u8 ep_num);
void		TPBulk_StallBulkEP						(void);

unsigned char		TPBulk_IsCBWValid							(unsigned char CBWsiz);
void		TPBulk_ErrorHandler						(unsigned char HostDevCase,u32_t DevXferCount);
unsigned long  	TPBulk_CheckDataTransfer				(u32_t deviceExpectSize, unsigned char deviceExpectDir);

void bulkonly_sendcsw(u8 ep_num, u8 status_csw);
void bulkonly_sendstall_cbw(void);

void TPBulk_ClassRequestHandler(void);
//void TPEDict_ClassRequestHandler (void);
void lock_system (void);
void unlock_system (void);

void SPC_BuildSenseData	(unsigned char key, unsigned char asc, unsigned char ascq);

#endif
