#include <common.h>
#include <drivers/usb.h>
#include <drivers/usb_bulkonly.h>


//#define WITH_FDU



bulk_only_in_server     bulk_only_in_now;
bulk_only_out_server    bulk_only_out_now; 
uchar		SPC_LUN;
CBW		TPBulk_CommandBlock;
CSW		TPBulk_CommandStatus;
u8*     mass_buf;
u32   lba_offset;   
u32   buf_offset; 
static REQUEST_SENSE_DATA request_sense_data;
u32  buffer_size;
USBFLAGS		UsbFlags;



static const INQUIRY_DATA StandardInquiryData =
{
	0x00,	0x00,
	0x00, 0x01,
	0x00,
	1, 0, 0, 0, 0,
	0x1F,
	0, 0,
	0,
	0, 0, 0, 0,
	"FMLDEV  ",
	"ExceedSpace     ",
	"0101",
	"ExceedSpace 2005~06 ",
	0,
	0,
	0x0238, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


/* send csw to host */ 
void bulkonly_sendcsw(u8 ep_num, u8 status_csw)
{   
    setindex(ep_num);
    TPBulk_ErrorHandler (status_csw, TPBulk_CommandBlock.dCBW_DataXferLen); 
    usb_bulk_loadfifo((u8*)&TPBulk_CommandStatus, sizeof(TPBulk_CommandStatus), BULK_EP_NUM);
    set_shortreg(R_USB_TxCSR,TXCSR_TxPktRdy);
    bulk_only_in_now.trans_status = ALL_TRANS_OVER;
    UsbFlags.command_state = USBFSM_CSWPROC;
}

/* CBW HANDLER */
// cbw error stall
void bulkonly_sendstall_cbw(void)
{
        TPBulk_ErrorHandler (CASEFAIL, 0);
		SPC_BuildSenseData (SD_ILLEGALREQUEST, 0x20, 0);
		TPBulk_StallBulkInEP ();
		UsbFlags.command_state = USBFSM_STALL;
}

unsigned char TPBulk_IsCBWValid( unsigned char CBWsiz)
{
	
	if(	CBWsiz == MS_CBW_SIZE \
		&& TPBulk_CommandBlock.dCBW_Signature == MS_CBW_SIGNATURE \
		&& TPBulk_CommandBlock.bCBW_LUN == 0 \
		&& TPBulk_CommandBlock.bCBW_CDBLen <= MAX_CDBLEN )
	{
       // puts("ok");
		return TRUE;
	}
	else{
        #ifdef USB_DEBUG
            printf ("cbw status=%d\n", -1);
        #endif
        
        return FALSE;
        }
	  
}





int TPBulk_CBWHandler( void )
{
	unsigned char CBWsize;
	CDB_SPC *cdb;
	u8_t		status;
    u8      mark;
	int		ret = USBERRCODE_SUCCESS;
    
 /* Get CBW */
	CBWsize = (uchar)usb_bulk_unloadfifo((unsigned char *)&TPBulk_CommandBlock, sizeof(TPBulk_CommandBlock), BULK_EP_NUM);
  

    
    mark = TPBulk_IsCBWValid(CBWsize);
    
	if(mark)
	{
		cdb = (PCDB_SPC)TPBulk_CommandBlock.CBW_CDB;				/* CBW_CDB contain the command block*/
		SPC_LUN = TPBulk_CommandBlock.bCBW_LUN;					/* Device Logical Unit Number */
		ControlData.dwLength = TPBulk_CommandBlock.dCBW_DataXferLen;
        bulk_only_in_now.dcswtag = TPBulk_CommandBlock.bCBW_Flag; // fill mark structure
		status = SPC_Handler (cdb, (u8_t)(TPBulk_CommandBlock.bCBW_Flag & 0x80), TPBulk_CommandBlock.dCBW_DataXferLen);
        #ifdef USB_DEBUG
        //printf ("SPC_Handler status=%d\n", status);
        #endif
        return ret;
	 
	}
	else
	{
		
     	// for Invalid CBW
		// Stall Both Bulk Endpoints
		// Let host issue reset recovery
		TPBulk_ErrorHandler (CASECBW, 0);
		ret = USBERRCODE_CASECBW;
	}

    return ret;
}


/* clr bulk in pipe */

void TPBulk_ClearBulkInEP(u8 ep_num)
{
	setindex (ep_num);

	printf ("TPBulk_ClearBulkInEP\n");
	//printf ("R_TxCSR=0x%04x\n", usb_ctrl->R_TxCSR);

	if(check_u16reg_assert(*R_USB_TxCSR, TXCSR_SentStall))
        clr_shortreg(R_USB_TxCSR, TXCSR_SentStall);

	if(check_u16reg_assert(*R_USB_TxCSR, TXCSR_UnderRun))
		clr_shortreg(R_USB_TxCSR, TXCSR_UnderRun);

    clr_shortreg(R_USB_TxCSR, TXCSR_SendStall);
    set_shortreg(R_USB_TxCSR, TXCSR_ClrDataTog);



	while(check_u16reg_assert(*R_USB_TxCSR, TXCSR_ClrDataTog));
  
	usb_EP0_transmit (0, 0);

    UsbFlags.command_state = USBFSM_STALL;

    // send  csw to host
    bulkonly_sendcsw(ep_num, CASEFAIL);
            

}


/* clr bulk out pipe */

void TPBulk_ClearBulkOutEP(u8 ep_num)
{


	printf ("TPBulk_ClearBulkOutEP\n");
    setindex (ep_num);


	// RX EP
    clr_shortreg(R_USB_RxCSR, RXCSR_SendStall);
    clr_shortreg(R_USB_RxCSR, RXCSR_SentStall);

    set_shortreg(R_USB_RxCSR, RXCSR_ClrDataTog);
   
    usb_EP0_transmit (0, 0);


    UsbFlags.command_state = USBFSM_STALL;
    // send  csw to host
    bulkonly_sendcsw(ep_num, CASEFAIL);
            

}




void TPBulk_ClassRequestHandler(void)
{
	unsigned char   c;

//	printf ("TPBulk_ClassRequestHandler bRequest=%d, wIndex=%d\n", ControlData.DeviceRequest.bRequest, ControlData.DeviceRequest.wIndex);
    
    // ADSC only supports host to device
    // It follows CSR architecture
    
	if(	ControlData.DeviceRequest.bRequest == USB_CLASS_REQUEST_USBSTOR_RESET 
		&& ControlData.DeviceRequest.wValue == 0 
		&& ControlData.DeviceRequest.wLength == 0 
		&& !(ControlData.DeviceRequest.bmRequestType & (unsigned char)USB_ENDPOINT_DIRECTION_MASK) )
	{
		if(ControlData.DeviceRequest.wIndex == 1 )
		{
			printf ("SOFT_RESET");
			TPBulk_SoftReset();
			usb_EP0_transmit(0,0);
		}
		else if (ControlData.DeviceRequest.wIndex == 0 )
		{
			printf ("Bulkonly Mass Storage RESET");
			TPBulk_HardReset();
			usb_EP0_transmit(0,0);
		}
		else
		{
			printf ("TPBulk_ClassRequestHandler stallEP0\n");
      	TPBulk_StallEP0();
		}
	}
	else if(ControlData.DeviceRequest.bRequest == USB_CLASS_REQUEST_USBSTOR_GETMAXLUN 
		&&   ControlData.DeviceRequest.wValue == 0 
		&&   ControlData.DeviceRequest.wIndex == 0 
		&&   ControlData.DeviceRequest.wLength == 1 
		&&   ControlData.DeviceRequest.bmRequestType & (unsigned char)USB_ENDPOINT_DIRECTION_MASK)
	{
		printf ("TPBulk_GetMaxLUN\n");
		c = TPBulk_GetMaxLUN();
		usb_EP0_transmit (&c,1);
	}
	else
	{
		printf ("UnSupport Class REQ");
		TPBulk_StallEP0();
	}
}

/*
void TPEDict_ClassRequestHandler (void)
{
	if(ControlData.DeviceRequest.bRequest & USB_REQUEST_MASK)
	{
		if(ControlData.DeviceRequest.wLength != 0)
            {
            UsbFlags.command_state = USBFSM_STALL;
            usb_StallEP0 ();
        }
            else
			TPEDict_Reset ();
	}
}


void TPEDict_Reset (void)
{
	UsbFlags.command_state = USBFSM_IDLE;
	memset (&TPEDict_CBW, 0, sizeof(TPEDict_CBW));
	memset (&TPEDict_CSW, 0, sizeof(TPEDict_CSW));
	usb_EP0_transmit (NULL, 0);
}
*/

/**************************************************/
/*  SoftReset                                     */
/*  Clear all buffers                             */
/*  Reset interface                               */
/*                                                */
/*  No affecting the state of the device          */
/**************************************************/
void TPBulk_SoftReset(void)
{
//	printf ("TPBulk_SoftReset\n");
	TPBulk_HardReset();

   

	UsbFlags.control_state = 0;
	UsbFlags.command_state = USBFSM_IDLE;
	memset (&ControlData, 0, sizeof(ControlData)); 
	
  
}


/**************************************************/
/*  HardReset                                     */
/*  Clear all buffers                             */
/*  Reset interface                               */
/*                                                */
/*  No changing on STALL or  toggle conditions.   */
/**************************************************/
void TPBulk_HardReset(void)
{
//	printf ("TPBulk_HardReset\n");
   
   usb_EPx_clear_fifo (BULK_EP_NUM);
 
}

/**************************************************/
/*  TPBulk_GetMaxLUN                              */
/*                                                */
/*                                                */
/*                                                */
/*                                                */
/**************************************************/
unsigned char TPBulk_GetMaxLUN(void)
{
	return 0;
}


/**************************************************/
/*  TPBulk_StallEP0                               */
/*                                                */
/*                                                */
/*                                                */
/*                                                */
/**************************************************/
void TPBulk_StallEP0(void)
{
	usb_StallEP0();
}

void usb_StallBulkEP()
{
    setindex (BULK_EP_NUM);
    set_shortreg(R_USB_RxCSR, RXCSR_SendStall);
    set_shortreg(R_USB_TxCSR, TXCSR_SendStall);
}
void TPBulk_StallBulkOutEP (void)
{

	setindex (BULK_EP_NUM);
	// RX EP
    set_shortreg(R_USB_RxCSR, RXCSR_SendStall);
    printf("TPBulk_StallBulkOutEP\n");
 
}

void TPBulk_StallBulkInEP (void)
{
   
	printf ("TPBulk_StallBulkInEP\n");

	setindex (BULK_EP_NUM);
	// TX EP
    set_shortreg(R_USB_TxCSR, TXCSR_SendStall);

}



/**************************************************/
/*  TPBulk_ErrorHandler                           */
/*                                                */
/*                                                */
/*                                                */
/*                                                */
/**************************************************/
void TPBulk_ErrorHandler (unsigned char HostDevCase, u32_t DevXferCount)
{
	switch(HostDevCase)
	{
		case CASEOK:
			TPBulk_CommandStatus.dCSW_DataResidue = TPBulk_CommandBlock.dCBW_DataXferLen - DevXferCount;
			TPBulk_CommandStatus.bCSW_Status = CSW_GOOD;
			break;

		case CASE1:     /* Hn=Dn*/
		case CASE6:     /* Hi=Di*/
		case CASE12:    /* Ho=Do*/
			TPBulk_CommandStatus.dCSW_DataResidue = 0;
			TPBulk_CommandStatus.bCSW_Status = CSW_GOOD;
			break;

		case CASE4:     /* Hi>Dn*/
		case CASE5:     /* Hi>Di*/
			usb_StallBulkEP();
			TPBulk_CommandStatus.dCSW_DataResidue = TPBulk_CommandBlock.dCBW_DataXferLen - DevXferCount;
			TPBulk_CommandStatus.bCSW_Status = CSW_FAIL;
			break;

		case CASE9:     /* Ho>Dn*/
		case CASE11:    /* Ho>Do*/
			usb_StallBulkEP();
			TPBulk_CommandStatus.dCSW_DataResidue = TPBulk_CommandBlock.dCBW_DataXferLen - DevXferCount;
			TPBulk_CommandStatus.bCSW_Status = CSW_FAIL;
			break;

		case CASE2:     /* Hn<Di*/
		case CASE3:     /* Hn<Do*/
			usb_StallBulkEP();
			TPBulk_CommandStatus.dCSW_DataResidue = 0;
			TPBulk_CommandStatus.bCSW_Status = CSW_PHASE_ERROR;
			break;

		case CASE8:     /* Hi<>Do */
			usb_StallBulkEP();
			TPBulk_CommandStatus.dCSW_DataResidue = TPBulk_CommandBlock.dCBW_DataXferLen;
			TPBulk_CommandStatus.bCSW_Status = CSW_PHASE_ERROR;
			break;

		case CASE10:    /* Ho<>Di */
			usb_StallBulkEP();
			TPBulk_CommandStatus.dCSW_DataResidue = TPBulk_CommandBlock.dCBW_DataXferLen;
			TPBulk_CommandStatus.bCSW_Status = CSW_PHASE_ERROR;
			break;

		case CASE7:     /* Hi<Di*/
			usb_StallBulkEP();
			TPBulk_CommandStatus.dCSW_DataResidue = -1; /* return non-zero value*/
			TPBulk_CommandStatus.bCSW_Status = CSW_PHASE_ERROR;
			break;

		case CASE13:    /* Ho<Do*/
			usb_StallBulkEP();
			TPBulk_CommandStatus.dCSW_DataResidue = -1; /* return non-zero value*/
			TPBulk_CommandStatus.bCSW_Status = CSW_PHASE_ERROR;
			break;

		case CASECBW:   /* invalid CBW */
			printf ("Invalid CBW");
			usb_StallBulkEP();
			TPBulk_CommandStatus.bCSW_Status = CSW_PHASE_ERROR;
			break;

		case CASEFAIL:	/* command failure */
			TPBulk_CommandStatus.dCSW_DataResidue = DevXferCount;
			TPBulk_CommandStatus.bCSW_Status = CSW_FAIL;
			break;

		default:
		    printf("Fatal error !");
			break;
	}

	TPBulk_CommandStatus.dCSW_Signature = MS_CSW_SIGNATURE;
	TPBulk_CommandStatus.dCSW_Tag = TPBulk_CommandBlock.dCBW_Tag;
}


unsigned long TPBulk_CheckDataTransfer (u32_t deviceExpectSize, unsigned char deviceExpectDir)
{
	if (TPBulk_CommandBlock.dCBW_DataXferLen == 0)	// host expect no data transfer or receive
	{
		if(deviceExpectSize != 0)
		{			// Hn < Di or Hn < Do
			printf("Hn < Di or Hn < Do");
			TPBulk_ErrorHandler (CASE2, 0);			// STALL Bulk-IN Pipe								
			TPBulk_SetBOTState (USBFSM_STALL);
			return FALSE;
		}
		else
		{			// Hn, Dn
			printf("Hn Dn");
			TPBulk_ErrorHandler (CASE1, 0);
			return TRUE;
		}
	}
	else if(TPBulk_CommandBlock.bCBW_Flag & USB_ENDPOINT_DIRECTION_MASK)	// host expects to receive data from the device
	{	
		if(deviceExpectSize == 0)	
		{			// Hi > Dn
			// stall Bulk-In Pipe
			printf("Hi > Dn ");
			TPBulk_ErrorHandler (CASE4, 0);
			TPBulk_SetBOTState (USBFSM_STALL);
			return FALSE;
		}
		else if(deviceExpectDir & USB_ENDPOINT_DIRECTION_MASK)		// device expects to transfer data to the host
		{
			if(TPBulk_CommandBlock.dCBW_DataXferLen == deviceExpectSize)
			{			// Hi = Di
//				USB_PRINT ("Hi = Di");
				TPBulk_ErrorHandler (CASE6, deviceExpectSize);
				return TRUE;
			}
			else if(TPBulk_CommandBlock.dCBW_DataXferLen > deviceExpectSize)
			{			// Hi > Di
				// here transfer actual size data to host
				printf("Hi > Di");
				TPBulk_ErrorHandler (CASE5, deviceExpectSize);
				TPBulk_SetBOTState (USBFSM_STALL);
				return FALSE;
			}
			else
			{			// Hi < Di
				// device want to transfer more data than host expect
				printf("Hi < Di");
				TPBulk_ErrorHandler (CASE7, deviceExpectSize);
				TPBulk_SetBOTState (USBFSM_STALL);
				return FALSE;
			}
		}
		else
		{			// Hi <> Do
			printf("Hi <> Do");
			TPBulk_ErrorHandler (CASE8, deviceExpectSize);
			TPBulk_SetBOTState (USBFSM_STALL);
			return FALSE;
		}
	}
	else
	{	// Ho - Host expects to send data to the device
		if(deviceExpectSize == 0)
		{
			// Ho > Dn
			printf("Ho > Dn");
			TPBulk_ErrorHandler (CASE9, deviceExpectSize);
			TPBulk_SetBOTState (USBFSM_STALL);
			return FALSE;
		}
		else if(TPBulk_CommandBlock.dCBW_DataXferLen > deviceExpectSize)
		{
			// Ho > Do
			printf("Ho > Do");
			TPBulk_ErrorHandler (CASE11, deviceExpectSize);
			TPBulk_SetBOTState (USBFSM_STALL);
			return FALSE;
		}
		else if(TPBulk_CommandBlock.dCBW_DataXferLen == deviceExpectSize)
		{	
			// Ho = Do
//			USB_PRINT ("Ho = Do");
			TPBulk_ErrorHandler (CASE12, deviceExpectSize);
			return TRUE;
		}
		else if(TPBulk_CommandBlock.dCBW_DataXferLen < deviceExpectSize)
		{
			// Ho < Do
			printf("Hn < Do");
			TPBulk_ErrorHandler (CASE13, deviceExpectSize);
			TPBulk_SetBOTState (USBFSM_STALL);
			return FALSE;
		}
		else
		{
			// Ho <> Di
			printf("Ho <> Di");
			TPBulk_ErrorHandler (CASE10, deviceExpectSize);
			TPBulk_SetBOTState (USBFSM_STALL);
			return FALSE;
		}
	}

	// return false indicate skip execute command
}

void		UsbEventNotify		(NTFEVENT ntfEvent, NTFTYPE ntfType)
{
    // reserved
}

void lock_system (void)
{
    // reserved
}

void unlock_system (void)
{
    // reserved
}
/***************************************************************************
 *          SPC handler ,based on UFI command set                          *
 ***************************************************************************/
 

void SPC_BuildSenseData	(unsigned char key, unsigned char asc, unsigned char ascq)
{
	memset (&request_sense_data, 0, sizeof(request_sense_data));

	request_sense_data.ResponseCode			= 0x70;
	request_sense_data.AdditionalSenseLen	= 0x12;
	request_sense_data.ASC						= asc;
	request_sense_data.ASCQ						= ascq;
	request_sense_data.SenseKey				= key;
}




unsigned char SPC_Handler(CDB_SPC *cdb, u8_t dir, u32_t transfer_length)
{
	unsigned char status = FALSE;
	unsigned char OpCode = ((PGENERIC_SPC)cdb)->OperationCode;
    if(OpCode != 0x2a && OpCode != 0x28 && OpCode != 0x00)

        printf ("SPC_Handler OpCode=0x%02x\n", OpCode);
 //   #ifdef USB_DEBUG
 //       printf ("SPC_Handler OpCode=0x%02x\n", OpCode);
 //   #endif

	switch(OpCode)
	{
		/* required command */
		case SPC_CMD_READ10:
			status = SPC_Read(cdb);
			break;

		case SPC_CMD_READCAPACITY:
			status = SPC_ReadCapacity(cdb);
			break;

   // case SPC_CMD_READFORMAT_CAPACITY:
   //    status = SPC_ReadFormat(cdb);
   //    break;
   //
	    case SPC_CMD_VERIFY10:
	    	status = SPC_Verify(cdb);
	    	break;

	    case SPC_CMD_WRITE10:
	    	status = SPC_Write (cdb);
         break;

		case SPC_CMD_INQUIRY:
			status = SPC_Inquiry (cdb);
			break;
      
	    case SPC_CMD_MODESENSE10:
	    	status = SPC_ModeSense (cdb);
	    	break;
      
	    case SPC_CMD_PRVENTALLOWMEDIUMREMOVAL:
	    	status = SPC_LockMedia (cdb);
	    	break;
		
	   case SPC_CMD_TESTUNITREADY:
	    	status = SPC_TestUnit (cdb);
	    	break;

		case SPC_CMD_REQUESTSENSE:
			status = SPC_RequestSense (cdb);
			break;

		default:
		//	printf ("SPC_Handler Exception\n");
			TPBulk_ErrorHandler (CASEFAIL, transfer_length);
		    if(dir == 0)	// out
		   	TPBulk_StallBulkOutEP ();
		    else 
		    	TPBulk_StallBulkInEP ();
			SPC_BuildSenseData (SD_ILLEGALREQUEST, 0x24, 0);
            /*
            UsbFlags.command_state = USBFSM_STALL;
            usb_bulk_loadfifo(&TPBulk_CommandStatus, sizeof(TPBulk_CommandStatus), 1);
            set_shortreg(*(vu16 *)usb_mentor.R_TxCSR,0);
            bulk_only_in_now.trans_status = ALL_TRANS_OVER;
            */
            status = FALSE;
			break;
	}

//	printf ("SPC_Handler OpCode=0x%02x, status=%d\n", OpCode, status);
	return status;
}

#define	BLOCK_COUNT	64

/***********************************************************************************/
/*  Function	:	SPC Read Command                                               */
/***********************************************************************************/
unsigned char SPC_Read (CDB_SPC *cdb)
{
	u32_t			BlockLen;
	u32_t			dwBlockNo;
	PREAD_SPC   cdbRead = (PREAD_SPC) cdb;
	u8_t			ret = TRUE;
	
	dwBlockNo = cdbRead->LBA_3;
	dwBlockNo = dwBlockNo << 8;
	dwBlockNo = dwBlockNo + cdbRead->LBA_2;
	dwBlockNo = dwBlockNo << 8;
	dwBlockNo = dwBlockNo + cdbRead->LBA_1;
	dwBlockNo = dwBlockNo << 8;
	dwBlockNo = dwBlockNo + cdbRead->LBA_0;

	BlockLen = cdbRead->XferLength_1;
	BlockLen = BlockLen << 8;
	BlockLen = BlockLen + cdbRead->XferLength_0;

    #ifdef USB_DEBUG
        //printf ("SPC_Read dwBlockNo=%d, BlockLen=%d\n", dwBlockNo, BlockLen);
    #endif
    if( ((dwBlockNo + BlockLen) << 9) > (lba_offset << 9)){
    
     SPC_BuildSenseData (SD_ILLEGALREQUEST, 0x21, 0x00); // logical block address out of range    
     TPBulk_ErrorHandler (CASEFAIL, 0);
	 TPBulk_StallBulkInEP();
	 // no status stage
	 ret = FALSE;
     return ret;
    }
	if(TPBulk_CheckDataTransfer (BlockLen << 9, USB_IN) == FALSE)
	{
		SPC_BuildSenseData (SD_NOSENSE, 0, 0);
		ret = FALSE;
	}
	else if(BlockLen == 0)
	{
		TPBulk_ErrorHandler (CASEOK, 0);
		UsbFlags.command_state = USBFSM_CSWPROC;
	}
	else
	{
        // mmap a block of buffer,here we map 2048 bytes
        
        //while((mass_buf = usb_mem_dev->mmap(usb_mem_dev, 0, dwBlockNo)) == NULL);
        
        bulk_only_in_now.buffer_addr = mass_buf;    // sram addr
        bulk_only_in_now.data_left_len = (BlockLen << 9) ;
        bulk_only_in_now.data_already_trans = 0;               
        bulk_only_in_now.trans_status = TRANS_ING;                   
        bulk_only_in_now.offset_now = dwBlockNo;


        UsbFlags.command_state = USBFSM_DATAIN;
	   
        //buffer_size = 1 << buf_offset;
        
        setindex(BULK_EP_NUM);

        //enable_interrupts();
        //enable_irq(INT_AS3310_USB_DMA);  0116

        usb_mem_dev->read(usb_mem_dev, bulk_only_in_now.offset_now, bulk_only_in_now.buffer_addr, 1);
        clr_shortreg(R_USB_IntrTxE, BULK_EP_NUM);
        set_shortreg(R_USB_TxCSR, TXCSR_AutoSet);  // enable autoset
        set_shortreg(R_USB_TxCSR, TXCSR_DMAReqEnab);  // enable dma req
        set_shortreg(R_USB_TxCSR, TXCSR_DMAReqMode);  // dma mode 1

        
        if(bulk_only_in_now.data_left_len > buffer_size){
            bulk_only_in_now.data_last_trans = buffer_size;
            bulk_only_in_now.data_left_len -= buffer_size;
            bulk_only_in_now.offset_now += (buffer_size >> 9);
            outx(buffer_size, R_USB_DmaCount);
        }

        else{
            bulk_only_in_now.data_last_trans = bulk_only_in_now.data_left_len;
            outx(bulk_only_in_now.data_left_len, R_USB_DmaCount);
            bulk_only_in_now.data_left_len = 0;
        }

        usb_dma_status = DMA_USB_TX_MULTIPLE;
        outx((u32)bulk_only_in_now.buffer_addr, R_USB_DmaAddr);
        inx(R_USB_DmaCtrl) &= 0xff0;  // clear dma ctrl reg except for burst mode and ep num
                                      // see usb.c ms_usb_config(): 
                                      // /* set dma control register : burst mode 1, ep num bulk_ep_num */

        inx(R_USB_DmaCtrl) |= 0xe;
       
        
        
        set_intreg(R_USB_DmaCtrl , CNTL_DMAEN);
        #ifdef     USB_DEBUG
           // printf("dma_ctrl = 0x%x dma_addr = 0x%x dma_count = 0x%x\n",*R_USB_DmaCtrl,
           //  *R_USB_DmaAddr,*R_USB_DmaCount);
        #endif    
        ret = TRUE;
		 
	}

	
	return ret;
}

/************************************************************************************/
/*  Function	:	SPC Write Command																	*/
/************************************************************************************/
unsigned char SPC_Write (CDB_SPC *cdb)
{
	u32_t			BlockLen;
	u32_t			dwBlockNo;
	u8_t			ret = TRUE;
	PWRITE_SPC	cdbWrite = (PWRITE_SPC) cdb;

    setindex(BULK_EP_NUM);
    //printf ("R_RxCSR=0x%04x\n", *(vu16 *)usb_mentor.R_RxCSR);

	dwBlockNo = cdbWrite->LBA_3;
	dwBlockNo = dwBlockNo << 8;
	dwBlockNo = dwBlockNo + cdbWrite->LBA_2;
	dwBlockNo = dwBlockNo << 8;
	dwBlockNo = dwBlockNo + cdbWrite->LBA_1;
	dwBlockNo = dwBlockNo << 8;
	dwBlockNo = dwBlockNo + cdbWrite->LBA_0;

	BlockLen = cdbWrite->XferLength_1;
	BlockLen = (BlockLen << 8);
	BlockLen = (BlockLen + cdbWrite->XferLength_0);
    #ifdef USB_DEBUG
       // printf ("SPC_Write dwBlockNo=%d, BlockLen=%d\n", dwBlockNo, BlockLen);
    #endif
	

    if( ((dwBlockNo + BlockLen) << 9) > (lba_offset << 9)){
        puts("out of range!\n");
        
     bulkonly_sendstall_cbw();
	 // no status stage
	 ret = FALSE;
     return ret;
    }

	if(BlockLen == 0)
	{
		//UsbFlags.command_state = USBFSM_CSWPROC;
		SPC_BuildSenseData (SD_NOSENSE, 0, 0);
		ret = TRUE;
	}
	else
	{
       UsbFlags.command_state = USBFSM_DATAOUT; // should set flag
       // while((mass_buf = usb_mem_dev->mmap(usb_mem_dev, 0, dwBlockNo)) == NULL);

       bulk_only_out_now.buffer_addr = mass_buf;    // attribute from sram, look up devmem device 
       bulk_only_out_now.data_last_got = 0;
       bulk_only_out_now.data_left_len =( BlockLen << 9 );
       bulk_only_out_now.offset_now = dwBlockNo;
       bulk_only_out_now.trans_status = TRANS_ING;
      
      

     
      // puts("multiple packets\n");
       clr_shortreg(R_USB_IntrRxE, BULK_EP_NUM); // disable ep intr of rx

       set_shortreg(R_USB_RxCSR, RXCSR_AutoClear);      // auto clear
       set_shortreg(R_USB_RxCSR, RXCSR_DMAReqEnab);     // dma req enable
       clr_shortreg(R_USB_RxCSR, RXCSR_DMAReqMode);     // dma reqmode 0
       
       //enable_interrupts();
       //enable_irq(INT_AS3310_USB_DMA); 0116
       
       //printf("data left size1:0x%x\n",*(vu16 *)usb_mentor.R_RxCount);
       //printf("dma_ctrl = 0x%x dma_addr = 0x%x dma_count = 0x%x\n",*(vu32 *)USB_DMA_CNTL,
       //       *(vu32 *)USB_DMA_AHB_ADDR,*(vu32 *)USB_DMA_BYTE_COUNT);

       if(bulk_only_out_now.data_left_len >= buffer_size)
       {
           bulk_only_out_now.data_last_got = buffer_size;
           bulk_only_out_now.data_left_len -= buffer_size;

          // printf("data_left_len =%d\n",bulk_only_out_now.data_left_len);
           outx(buffer_size, R_USB_DmaCount);
         
       }
       else{
           bulk_only_out_now.data_last_got = bulk_only_out_now.data_left_len;
           outx(bulk_only_out_now.data_left_len, R_USB_DmaCount);  
           bulk_only_out_now.data_left_len = 0;
       }

        usb_dma_status = DMA_USB_RX;
        outx((u32)bulk_only_out_now.buffer_addr, R_USB_DmaAddr);  // mass_buf, got from mmap, actually is attributed from sram
        inx(R_USB_DmaCtrl) &= 0xff0;  // clear dma ctrl reg except for burst mode and ep num
                                      // see usb.c ms_usb_config(): 
                                      // /* set dma control register : burst mode 1, ep num bulk_ep_num */
        
        inx(R_USB_DmaCtrl) |= 0xc;
        
        clr_intreg(R_USB_DmaCtrl , CNTL_DIC);
        
        set_intreg(R_USB_DmaCtrl , CNTL_DMAEN);
        #ifdef     USB_DEBUG
             // printf("dma_ctrl = 0x%x dma_addr = 0x%x dma_count = 0x%x\n",*R_USB_DmaCtrl,
             //  *R_USB_DmaAddr,*R_USB_DmaCount);
        #endif   
       ret = TRUE;

    }
    
	return ret;
}




unsigned char SPC_ReadCapacity (CDB_SPC *cdb)
{
	READ_CAPACITY_SPC *cdbReadCapacity = (READ_CAPACITY_SPC *)cdb;
	READ_CAPACITY_DATA *data;
	unsigned int size, bsize;
	u8_t ret = TRUE;

	SPC_BuildSenseData (SD_NOSENSE, 0, 0);

	// working draft Small Computer System Interface ¡ª 2 (X3T9.2/375R revision 10L)
	// P182

	//	The logical block address shall be zero if the PMI bit is zero. 
	//	If the PMI bit is zero and the logical block address is not zero, 
	//	the target shall return a CHECK CONDITION status, the sense key shall be set to ILLEGAL REQUEST and
	//	the additional sense code set to ILLEGAL FIELD IN CDB.
	//
	//	A partial medium indicator (PMI) bit of zero indicates that the returned logical block address 
	//	and the block length in bytes are those of the last logical block on the logical unit.

	// A PMI bit of one indicates that the returned logical block address and block length in bytes 
	//		are those of the logical block address after which a substantial delay in data transfer 
	//		will be encountered. 
	//	This returned logical block address shall be greater than or equal to the logical block address
	//		specified by the RelAdr and logical block address fields in the command descriptor block.

	// A relative address (RelAdr) bit of one indicates that the logical block address field 
	//	is a two¡¯s complement displacement.
	//	This negative or positive displacement shall be added to the logical block address last accessed 
	//	on the logical unit to form the logical block address for this command. 
	//	This feature is only available when linking commands. 
	// The feature requires that a previous command in the linked group have accessed a block of data 
	//	on the logical unit.
	//	A RelAdr bit of zero indicates that the logical block address field specifies 
	//	the first logical block of the range of logical blocks to be operated on by this command.

 

	if(cdbReadCapacity->PMI == 0)
	{
		if (	cdbReadCapacity->LBA_0 
			|| cdbReadCapacity->LBA_1
			|| cdbReadCapacity->LBA_2
			|| cdbReadCapacity->LBA_3)
		{
            //		printf ("SPC_ReadCapacity exception\n");
			TPBulk_ErrorHandler (CASEFAIL, 0);
			TPBulk_StallBulkInEP ();
			SPC_BuildSenseData (SD_ILLEGALREQUEST, ASC_INVALID_FIELD_IN_CDB, 0x00);
			UsbFlags.command_state = USBFSM_STALL;
			return FALSE;
		}
	}
	else
	{
        //		printf ("SPC_ReadCapacity exception, PMI = 1\n");
	}

    bsize = BLOCK_SIZE; // ### should get from nand device
                        // but right now we make it 512 temporarily
    //size = usb_mem_dev_i->lba_size;       
    //size >>= 9;
    //size = size-1;
    size = lba_offset - 1;

	data = (READ_CAPACITY_DATA *)&ControlData.DataBuffer;
	data->BlockLength_3 = (bsize >> 24) & 0xFF;
	data->BlockLength_2 = (bsize >> 16) & 0xFF;
	data->BlockLength_1 = (bsize >> 8)  & 0xFF;
	data->BlockLength_0 = bsize & 0xFF;        
	data->LogicalBlockAddress_3 = (size >> 24) & 0xFF;
	data->LogicalBlockAddress_2 = (size >> 16) & 0xFF;
	data->LogicalBlockAddress_1 = (size >> 8)  & 0xFF;
	data->LogicalBlockAddress_0 = size & 0xFF;

	if(	(TPBulk_CommandBlock.bCBW_Flag & 0x80) == 0 
		|| TPBulk_CommandBlock.dCBW_DataXferLen < sizeof(READ_CAPACITY_DATA))
	{
        //puts("#");
		TPBulk_ErrorHandler (CASECBW,0);
		SPC_BuildSenseData (SD_UNITATTENTION, 0x20, 0);
		UsbFlags.command_state = USBFSM_STALL;
		TPBulk_StallBulkInEP ();
		ret = FALSE;
	}
	else
	{
		TPBulk_ErrorHandler (CASEOK, sizeof(READ_CAPACITY_DATA));
        usb_bulk_loadfifo((unsigned char *)data, sizeof(READ_CAPACITY_DATA), BULK_EP_NUM);
        bulk_only_in_now.trans_status = TRANS_OVER;
        UsbFlags.command_state = USBFSM_CSW;
        set_shortreg(R_USB_TxCSR, TXCSR_TxPktRdy);
        ret = TRUE;
	}

	return ret;
}

unsigned char SPC_Verify (CDB_SPC *cdb)
{
    /*
	TPBulk_ErrorHandler (CASEFAIL, 0);
	SPC_BuildSenseData (SD_ILLEGALREQUEST, 0x20, 0);
	UsbFlags.command_state = USBFSM_STALL;
	TPBulk_StallBulkInEP ();
	return FALSE;    
    */
    SPC_BuildSenseData(SD_NOSENSE, 0 ,0);
    bulkonly_sendcsw(BULK_EP_NUM, CASEOK);
    return TRUE;
}

/*
unsigned char SPC_ReadFormat (CDB_SPC *cdb)
{
    unsigned char *tmpp;
    PCapacityListHeader_SPC caph;
    PFormat_CapacityDesc_SPC fcadesc;
    caph->reserved0 = 0;
    caph->reserved1 = 0;
    caph->reserved2 = 0;
    caph->CapacityListLen = 8;

    #ifdef WITHOUT_FDU

    UsbFlags.command_state = USBFSM_CBWPROC ;
    fcadesc->Desc_Code = NOCARTRIDGE;
    fcadesc->NumOfBlocks_3 = 0;
    fcadesc->NumOfBlocks_2 = 0;
    fcadesc->NumOfBlocks_1 = 0x0b; 
    fcadesc->NumOfBlocks_0 = 0x40;
    fcadesc->BlockLen_2 = 0;
    fcadesc->BlockLen_1 = 0x02;
    fcadesc->BlockLen_0 = 0x00;
    memcpy( tmpp,caph, sizeof(CapacityListHeader));
    memcpy( tmpp + sizeof(CapacityListHeader), fcadesc, 8);
    TPBulk_ErrorHandler (CASEOK, TPBulk_CommandBlock.dCBW_DataXferLen);
    usb_bulk_loadfifo(tmpp, 12, 1);
    bulk_only_in_now.trans_status = TRANS_OVER;
    setindex(1);
    set_shortreg(*(vu16 *)usb_mentor.R_TxCSR,0);
    #endif
    return TRUE;


}
*/

unsigned char SPC_Inquiry (CDB_SPC *cdb)
{
	PINQUIRY_SPC  cdbInquiry = (PINQUIRY_SPC) cdb;	
	u8_t	ret = TRUE;

	if(cdbInquiry->EVPD == 0 && cdbInquiry->CMDDT == 0 )    // EVPD should be 0
	{
		if(cdbInquiry->PageCode != 0)		// PageCode must be zero required by SPC2
		{
			bulkonly_sendstall_cbw();
			ret = FALSE;
		}

		TPBulk_ErrorHandler (CASEOK, TPBulk_CommandBlock.dCBW_DataXferLen);
        usb_bulk_loadfifo((unsigned char *)&StandardInquiryData, TPBulk_CommandBlock.dCBW_DataXferLen, BULK_EP_NUM);
        bulk_only_in_now.trans_status = TRANS_OVER;
        setindex(BULK_EP_NUM);
        set_shortreg(R_USB_TxCSR,TXCSR_TxPktRdy);
		UsbFlags.command_state = USBFSM_CBWPROC ;
		ret = TRUE;
	}
  
	else
	{
		bulkonly_sendstall_cbw();
		ret = FALSE;
	}

	if(ret == TRUE)
	{
		UsbEventNotify	((NTFEVENT)NTFEVENT_USB_UNLOCK, NTFTYPE_TASK);
	}

	return ret;
}


unsigned char SPC_ModeSense (CDB_SPC *cdb)
{
	MODE_SENSE_SPC *cdbModeSense = (MODE_SENSE_SPC *)cdb;
	unsigned char sense_data[128];

	if(cdbModeSense->Reserved0 != 0 || cdbModeSense->Reserved1 != 0)
	{
		// unrecognized command parameter
	    bulkonly_sendstall_cbw();
		return FALSE;
	}

	if(cdbModeSense->PageCode == 0x1C && cdbModeSense->SubPageCode == 0)
	{
		// Informational Exceptions Control mode page
		MODE_PARAMETER_HEADER6 mode_parameter_header6;
		IEC_MODE_PAGE mode_page;
		uchar trans_len;

		mode_parameter_header6.MediumType = 0x00;
		mode_parameter_header6.ModeDataLength = 12;
		mode_parameter_header6.DeviceSpecificParameter = 0x00;
		mode_parameter_header6.BlockDescriptorLength = 0x00;

		memset (&mode_page, 0, sizeof(IEC_MODE_PAGE));
		mode_page.PageCode = 0x1C;
		mode_page.PageLength = 0x0A;
		mode_page.MRIE	= 0x01;	// Asynchronous event reporting:

		memcpy (sense_data, &mode_parameter_header6, 4);
		memcpy (sense_data + 4, &mode_page, 12);

		trans_len = 16;

		if(cdbModeSense->ParameterLen < 16)
			trans_len = cdbModeSense->ParameterLen;

		TPBulk_ErrorHandler (CASEOK, trans_len);

        usb_bulk_loadfifo((unsigned char *)sense_data, trans_len, BULK_EP_NUM);
        bulk_only_in_now.trans_status = TRANS_OVER;
        setindex(BULK_EP_NUM);
        set_shortreg(R_USB_TxCSR, TXCSR_TxPktRdy);
        UsbFlags.command_state = USBFSM_CBWPROC ;
		return TRUE;    
	}
	else if(cdbModeSense->PageCode == 0x3F && cdbModeSense->SubPageCode == 0)
	{
		MODE_PARAMETER_HEADER6 mode_parameter_header6;
		IEC_MODE_PAGE mode_page;
		RWER_MODE_PAGE rwer_mode_page;
		uchar trans_len;

		// return all mode page
		mode_parameter_header6.MediumType = 0x00;
		mode_parameter_header6.ModeDataLength = 24;
		mode_parameter_header6.DeviceSpecificParameter = 0x00;
		mode_parameter_header6.BlockDescriptorLength   = 0x00;

		memcpy (sense_data, &mode_parameter_header6, 4);

		// page 0x01
		// Read-Write Error Recovery mode page	
		memset (&rwer_mode_page, 0, sizeof(RWER_MODE_PAGE));
		rwer_mode_page.PageCode = 0x01;
		rwer_mode_page.PageLength = 0x0A;
		rwer_mode_page.ReadRetryCount = 0x03;
		rwer_mode_page.WriteRetryCount = 0x03;
		// A RECOVERY TIME LIMIT field set to zero specifies that the device server shall use its default value.
		rwer_mode_page.RecoveryTimeLimit = 0x00;
		memcpy (sense_data + 4, &rwer_mode_page, 12);

		memset (&mode_page, 0, sizeof(IEC_MODE_PAGE));
		mode_page.PageCode = 0x1C;
		mode_page.PageLength = 0x0A;
		mode_page.MRIE	= 0x01;	// Asynchronous event reporting:
		memcpy (sense_data + 16, &rwer_mode_page, 12);

		trans_len = 28;

		if(cdbModeSense->ParameterLen < trans_len)
			trans_len = cdbModeSense->ParameterLen;


		TPBulk_ErrorHandler (CASEOK, trans_len);

        usb_bulk_loadfifo((unsigned char *)sense_data, trans_len, BULK_EP_NUM);
        bulk_only_in_now.trans_status = TRANS_OVER;
        setindex(BULK_EP_NUM);
        set_shortreg(R_USB_TxCSR, TXCSR_TxPktRdy);
		UsbFlags.command_state = USBFSM_CBWPROC ;
		return TRUE;    
	}
	else
	{
		// unrecognized command parameter
        bulkonly_sendstall_cbw();
		return FALSE; 
	}
}

unsigned char SPC_LockMedia (CDB_SPC *cdb)
{
	MEDIA_REMOVAL_SPC *cdbMediaRemoval = (MEDIA_REMOVAL_SPC *) cdb;

   

//	printf ("SPC_LockMedia %s\n", cdbMediaRemoval->Prevent ? "Lock" : "Unlock");

	if(cdbMediaRemoval->Prevent == 0)
	{
		UsbEventNotify	((NTFEVENT)NTFEVENT_USB_UNLOCK, NTFTYPE_TASK);
		unlock_system ();
	}
	else
	{
		// lock system to prevent system destroy during USB transaction
		lock_system ();
		UsbEventNotify	((NTFEVENT)NTFEVENT_USB_LOCKED, NTFTYPE_TASK);
	}
    bulkonly_sendcsw(BULK_EP_NUM, CASEOK);
	return TRUE;    
}

unsigned char SPC_TestUnit (CDB_SPC *cdb)
{
	bulkonly_sendcsw(BULK_EP_NUM, CASEOK);
	UsbEventNotify	((NTFEVENT)NTFEVENT_USB_LINKACTIVE, NTFTYPE_TASK);
	return TRUE;    
}

unsigned char SPC_RequestSense (CDB_SPC *cdb)
{
	TPBulk_ErrorHandler (CASEOK, sizeof(REQUEST_SENSE_DATA));
	UsbFlags.command_state = USBFSM_CBWPROC ;

    usb_bulk_loadfifo((unsigned char *)&request_sense_data, sizeof(REQUEST_SENSE_DATA), BULK_EP_NUM);
    bulk_only_in_now.trans_status = TRANS_OVER;
    setindex(BULK_EP_NUM);
    set_shortreg(R_USB_TxCSR, TXCSR_TxPktRdy);
	//puts("ok,request_sense\n");
    return TRUE;    
}
