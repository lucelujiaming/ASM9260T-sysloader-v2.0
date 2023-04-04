#include <common.h>
#include <drivers/usb.h>
#include <drivers/usb_protocol.h>
#include <drivers/devmem.h>
#include <drivers/usb_bulkonly.h>
#include <drivers/nftl_mount.h>
#include <drivers/sd_mmc.h>

/* global variaties */
bulk_only_in_server     bulk_only_in_now;
bulk_only_out_server    bulk_only_out_now; 
unsigned char   usb_dma_status;
struct device * usb_mem_dev;        // for dma buffering
//struct devmem * usb_mem_dev_i;
u32   lba_offset;   
u32   buf_offset;
u32  buffer_size;
u8*     mass_buf;
USBFLAGS		UsbFlags; 
CONTROL_XFER ControlData;
static char SRAM ALIGN32 mem_usb_buffer[512];

/********************** usb descriptor list *********************/

static const USB_LANGID_DESCRIPTOR LangDescr = {
	sizeof(USB_LANGID_DESCRIPTOR),
	USB_STRING_DESCRIPTOR_TYPE,
	{0x0409	}		// Unicode 
};

/* FML Protocol Interface Descriptor */
 

static const USB_DEVICE_DESCRIPTOR fml_device_descriptor = {
	sizeof(USB_DEVICE_DESCRIPTOR),		// Size of this descriptor
	USB_DEVICE_DESCRIPTOR_TYPE,			// Device descriptor type 
	0x0110,										// USB version 1.1
	0x00,											// Device class code (vendor-specific, down/load)
	0,												// Sub class code
	0,												// Protocol code
	EP0_PACKET_SIZE,							// Size of Endpoint 0
	0x04b8,										// Vendor ID (epson)
	0xF012,										// Product ID (e-dict)
	0x0001,										// This device version
	0,												// Index of manufacturer
	0,												// Index of product
	0,												// Index of serial number
	1												// Number of configurations
};

// Configuration descriptor
static const USB_CONFIGURATION_DESCRIPTOR fml_configuration_descriptor = {
	9,												// Size of this descriptor		
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	// Configuration descriptor type
	9 + 9 + 7*2,								// Total configuration size
	//9 + 9*2 + 7*4,								// Total configuration size
	//2,												// Number of this interface
	1,												// Number of this interface
	1,												// SetConfiguration() argument value
	0,												// Index of string descriptor
	0x80,											// 
	0x32											// power	(100ma)
};


// Interface descriptor
static const USB_INTERFACE_DESCRIPTOR fml_interface_descriptor = {
	9,												// Size of this descriptor
	USB_INTERFACE_DESCRIPTOR_TYPE,		// Interface descriptor type	
	0,												// Number of this configuration
	0,												// Number of alternate interface		
	2,											// Number of endpoints uesd by this interface	
	0xFF,											// Interface class is vendor-specific.
	0x01,											// Subclass code	
	0xFF,											// vendor-specific protocol for this interface.
	0												// Index of string descriptor
};

// EPb Endpoint descriputor
static const USB_ENDPOINT_DESCRIPTOR endpoint_b_descriptor = {
	7,												// Size of this descriptor
	USB_ENDPOINT_DESCRIPTOR_TYPE,			// Endpoint descriptor type
	0x82,											// Endpoint address = 2,direction in
	USB_ENDPOINT_TYPE_BULK,					// Transfer type (BULK)
	BULK_PACKET_SIZE,							// Max packet size (64bytes)
	0												// Interval
};

// EPc Endpoint descriputor
static const USB_ENDPOINT_DESCRIPTOR endpoint_c_descriptor = {
	7,												// Size of this descriptor
	USB_ENDPOINT_DESCRIPTOR_TYPE,			// Endpoint descriptor type
	0x03,											// Endpoint address = 3,direction out
	USB_ENDPOINT_TYPE_BULK,					// Transfer type(bulk)
	BULK_PACKET_SIZE,							// Max packet size (64bytes)
	0												// Interval
};


/* MassStorage Protocol Interface eDescriptot */

// Device descriptor data
static const USB_DEVICE_DESCRIPTOR ms_device_descriptor = {
	sizeof(USB_DEVICE_DESCRIPTOR),		// Size of this descriptor
	USB_DEVICE_DESCRIPTOR_TYPE,			// Device descriptor type 
	0x0110,										// USB version 1.1
	0x00,											// Device class code (vendor-specific, down/load)
	0,												// Sub class code
	0,												// Protocol code
	EP0_PACKET_SIZE,							// Size of Endpoint 0
     
     
    USB_VENDOR_ID,										// Vendor ID (epson)
	USB_PRODUCT_ID,										// Product ID (e-dict)
	0x0001,										// This device version
	0,												// Index of manufacturer
	0,												// Index of product
	1,												// Index of serial number
	1												// Number of configurations
};

// Configuration descriptor
static const USB_CONFIGURATION_DESCRIPTOR ms_configuration_descriptor = {
	9,												// Size of this descriptor		
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	// Configuration descriptor type
	9 + 9 + 7*2,								// Total configuration size
	1,												// Number of this interface
	1,												// SetConfiguration() argument value
	0,												// Index of string descriptor
	0x80,											// 
	0x32											// power	(100ma)
};

static const USB_INTERFACE_DESCRIPTOR ms_interface_descriptor = {
	9,												// Size of this descriptor
	USB_INTERFACE_DESCRIPTOR_TYPE,		// Interface descriptor type	
	0x00,											// Number of this configuration
	0x00,											// Number of alternate interface		
	2,												// Number of endpoints uesd by this interface	
	0x08,											// Interface class is MassStorage
	0x04,											// Subclass code	(SCSI)
	0x50,											// BULK_ONLY protocol for this interface.
	0												// Index of string descriptor
};

// EPa Endpoint descriputor
static const USB_ENDPOINT_DESCRIPTOR endpoint_a_descriptor = {
	7,												// Size of this descriptor
	USB_ENDPOINT_DESCRIPTOR_TYPE,			// Endpoint descriptor type
	USB_TRANSMIT_INDEX | BULK_EP_NUM,											// Endpoint address = 1,direction in
	USB_ENDPOINT_TYPE_BULK,					// Transfer type (BULK)
	BULK_PACKET_SIZE,							// Max packet size (64bytes)
	0												// Interval
};

// EPc Endpoint descriputor
static const USB_ENDPOINT_DESCRIPTOR endpoint_d_descriptor = {
	7,												// Size of this descriptor
	USB_ENDPOINT_DESCRIPTOR_TYPE,			// Endpoint descriptor type
	USB_RECEIVE_INDEX | BULK_EP_NUM,											// Endpoint address = 4,direction out
	USB_ENDPOINT_TYPE_BULK,					// Transfer type(bulk)
	BULK_PACKET_SIZE,							// Max packet size (64bytes)
	0												// Interval
};

static const uchar ms_ManuDescr[]	= { 16, USB_STRING_DESCRIPTOR_TYPE,
												'F', 0, 'M', 0, 'L', 0, 'D', 0, 'E', 0, 'V', 0,  0,  0,
};

/****************************************************************/




/* if hs ,usb should make a resistant calibration here */

void hs_calibration(void)
{
   
	// Calibration 
	// DP
	printf("Start Calibration...\n");
	outl(0x000f0000, USB_TX + 4); // set TXCAL45DP = '1111'
	outl(0x00200000, USB_TX + 4); // enable TXCAL45DP
	delay(150);
	outl(0x00000080, USB_TX + 4); // enable TXCALIBRATE
	outl(0x00000080, USB_TX + 8); // disable TXCALIBRATE                                       // wait for analog stabilize
	while((inl(USB_TX) & 0x00800000)==0 && (inl(USB_TX) & 0x000f0000)!=0){
		inl(USB_TX) = inl(USB_TX) - 0x00010000; // decrement TXCAL45DP
		delay(100);
        outl(0x00000080, USB_TX + 4);   // enable TXCALIBRATE
        outl(0x00000080, USB_TX + 8);   // disable TXCALIBRATE 

          
		// wait for analog stabilize
	}
	
	if (inl(USB_TX) & 0x00800000) {
		printf("45DP Calibration finish! TXCAL45DP set to %x \n",(inl(USB_TX) & 0x000f0000)>>16);
	}else if ((inl(USB_TX) & 0x000f0000)==0) {
		printf("It has been the Maximum resistance for 45DP.\n");
	}
	outl(0x00200000, USB_TX + 8);  // disable TXCAL45DP
	
	// DN
    outl(0x00000f00, USB_TX + 4);   // set TXCAL45DN = '1111' 
    outl(0x00002000, USB_TX + 4);   // enable TXCAL45DN       

	
	delay(150);
	outl(0x00000080, USB_TX + 4); // enable TXCALIBRATE
	outl(0x00000080, USB_TX + 8); // disable TXCALIBRATE      
	while((inl(USB_TX) & 0x00800000)==0 && (inl(USB_TX) & 0x00000f00)!=0){
		inl(USB_TX) = inl(USB_TX) - 0x00000100; // decrement TXCAL45DP
		delay(100);
		outl(0x00000080, USB_TX + 4);   // enable TXCALIBRATE   
	}   outl(0x00000080, USB_TX + 8);   // disable TXCALIBRATE  
	
	if (inl(USB_TX) & 0x00800000) {
		printf("45DN Calibration finish! TXCAL45DN set to %x \n",(inl(USB_TX) & 0x00000f00)>>8);
	}else if ((inl(USB_TX) & 0x00000f00)==0) {
		printf("It has been the Maximum resistance for 45DN.\n");
	}

    outl(0x00002000, USB_TX + 8);   // disable TXCAL45DN
	 
	outl(0, USB_PWD);
	
}

/* soft disconnected and re-connect to get required speed */ 
void otg_init(void){
    //write 0 to hostdiscon bit, Client Mode , ARM version
	outl( 0x02000000, (USB_SYSCTRL + 8) );	

    #if CONFIG_USB_HIGH_SPEED
        //enable Soft Conn and High Speed
	    outb(0x60,R_USB_Power);
    #else
        //enable Soft Conn and Full Speed
        outb(0x40,R_USB_Power);
    #endif
}


/* enable usb intr */
void usb_interrupt_enable(void)
{
    outx(0xf7, R_USB_IntrUSBE); // USB INTR ENABLE
       
	//now enable ep intr
    outx(0x000f, R_USB_IntrTxE);
	outx(0x000e, R_USB_IntrRxE);       
}


/* usb ep0 transmit */
int usb_EP0_transmit (unsigned char *buf, u32_t len)
{
	u32_t	i;
	u32_t	to_transfer;
	u8_t	finish = 0;
	int	ret;
	vu8 *fifo_char;


	setindex (0);

    if( check_u16reg_assert(*R_USB_TxCSR, CSR0_SentStall) )
        clr_shortreg(R_USB_TxCSR, CSR0_SentStall);

    if( check_u16reg_assert(*R_USB_TxCSR, CSR0_SetupEnd) )
        clr_shortreg(R_USB_TxCSR, CSR0_SetupEnd);

   
    set_shortreg(R_USB_TxCSR, CSR0_ServicedRxPktRdy);
	// Set ServicedRxPktRdy, tell USB IP prepare to receive next packet
	// here don't care next packet in or out direction, which depend on the DEVICE REQUIST
    //set_shortreg(R_USB_TxCSR, CSR0_ServicedRxPktRdy);
    
	if(len == 0)
	{
        set_shortreg( R_USB_TxCSR, CSR0_DataEnd );
	 
        while(check_u16reg_deassert(*R_USB_IntrTx, INTRTXE_EP0)); 
        ret = 0;
        goto exit;
	}

	if(ControlData.dwLength > len)
		to_transfer = len;
	else
		to_transfer = ControlData.dwLength;

	ret = to_transfer;

	fifo_char = R_USB_FIFO0;

	while(!finish)
	{
		if(to_transfer >= EP0_PACKET_SIZE)
			i = EP0_PACKET_SIZE;
		else
		{
			i = to_transfer;
			finish = 1;
		}
		to_transfer -= i;	

		while(i)
		{
			*fifo_char = *buf;
			buf ++;
			i --;
		}
    	
		// Tell the USBC data ready
		if(finish)
		{
			// The CPU sets this bit (CSR0_DataEnd):
			//		1. When setting TxPktRdy for the last data packet.
			//		2. When clearing RxPktRdy after unloading the last data packet.
			//		3. When setting TxPktRdy for a zero length data packet. 
			//	It is cleared automatically.
            inx(R_USB_TxCSR) |= ( (1 << CSR0_TxPktRdy) | (1 << CSR0_DataEnd) );

		}
		else
            set_shortreg( R_USB_TxCSR, CSR0_TxPktRdy );
      

	    
		
		// waiting for packet transfered
		while(check_u16reg_assert(*R_USB_TxCSR, CSR0_TxPktRdy)){
			// The CPU sets this bit after loading a data packet into the
			//	FIFO. It is cleared automatically when a data packet has
			//	been transmitted. An interrupt is also generated at this
			// point (if enabled).

			if(check_u16reg_assert(*R_USB_TxCSR,CSR0_SetupEnd))
			{
				// exception of protocol violation
				// If the host prematurely ends a transfer by entering the STATUS phase before 
				//		all the data for the request has been transferred, 
				//	or by sending a new SETUP packet before completing the current transfer, 
				//		then the SetupEnd bit (CSR0.D4) will be set and an Endpoint 0 interrupt generated.
				ret = -1;

				printf ("exception CSR0_SetupEnd\n");
				goto exit;
			}
		}

 }

exit:

	if(ret < 0)
	{
		// exception handling
		usb_StallEP0 ();
	}

	UsbFlags.control_state = USB_IDLE;
    return ret;
}



/* usb epx read */
int usb_EPx_read (unsigned char *buf, unsigned char len, int ep)
{
	int i = 0;
    setindex (ep);

	if( len > inx(R_USB_RxCount) )
		len = inx(R_USB_RxCount);

	while(len)
	{
		*buf ++ = inb (R_USB_FIFO0 + ep * 4);
		i ++;
		len --;
	}
    if( ep == 0){
    
        set_shortreg (R_USB_TxCSR, CSR0_FlushFIFO);    // flush fifo
        //set_shortreg(R_USB_TxCSR, CSR0_ServicedRxPktRdy);   // clr RxPktRdy
    }
    else{

    
        set_shortreg (R_USB_RxCSR, RXCSR_FlushFIFO);    // flush fifo
	    clr_shortreg (R_USB_RxCSR, RXCSR_RxPktRdy);		// clr RxPktRdy
    }


	return i;
}

/* send a stall to ep0 piple */
void usb_StallEP0 (void)
{

	setindex (0);
    set_shortreg(R_USB_TxCSR, CSR0_SendStall); 
}


/* usb ep0 routine */
void usb_EP0_setup (void)
{
	volatile  unsigned short CSR0_reg;

	USBFLAGS		*pUsbFlags		= &UsbFlags;
	CONTROL_XFER	*pControlData	= &ControlData;

	setindex (0);


	CSR0_reg = inx(R_USB_TxCSR);
        //	printf ("usb_EP0_setup process 0x%04x\n", CSR0_reg);

	if ( check_u16reg_assert (CSR0_reg, CSR0_SentStall))		//Sent Stall ?
	{
        //		printf ("Sent Stall\n");
		clr_shortreg (R_USB_TxCSR, CSR0_SentStall);
		pUsbFlags->control_state = USB_IDLE;
	}

	if(check_u16reg_assert(CSR0_reg, CSR0_SetupEnd))	//Setup End ?
	{
        //		printf ("Setup End\n");
		set_shortreg(R_USB_TxCSR, CSR0_ServicedSetupEnd);
		pUsbFlags->control_state = USB_IDLE;
	}

	if (check_u16reg_assert(CSR0_reg, CSR0_RxPktRdy) ) 
	{
		// This bit is set when a data packet has been received. An
		// interrupt is generated when this bit is set. The CPU clears
		// this bit by setting the ServicedRxPktRdy bit.
		// RxPktRdy set?

		// Unload FIFO;
		int length = usb_EPx_read ((u8_t *)&pControlData->DeviceRequest , sizeof(DEVICE_REQUEST), EP0);

		// Set ServicedRxPktRdy, tell USB IP prepare to receive next packet
		// here don't care next packet in or out direction, which depend on the DEVICE REQUIST
        //		usb_ctrl->R_TxCSR |= CSR0_ServicedRxPktRdy;
       
		if (length != sizeof(DEVICE_REQUEST))
		{
            
			// the packet is too small, error
			//Set SendStall
            set_shortreg(R_USB_TxCSR, CSR0_SendStall);

            //			printf ("the packet is too small\n");
		}
		else
		{
			pControlData->dwLength = pControlData->DeviceRequest.wLength;
			pControlData->dwCount  = 0;

			if (pControlData->DeviceRequest.bmRequestType & (unsigned char)USB_ENDPOINT_DIRECTION_MASK)
			{
				/* device to host */
                //				printf ("device to host\n");
				pUsbFlags->setup_packet = 1;        // need to enter request handler
				pUsbFlags->control_state = USB_IDLE;
			}
			else
			{
                //				printf ("host to device\n");
				/* host to device */
				if (pControlData->DeviceRequest.wLength == 0)
				{
					pUsbFlags->setup_packet = 1;    // need to enter request handler
					pUsbFlags->control_state = USB_IDLE;
				}
				else
				{
					set_shortreg(R_USB_TxCSR, CSR0_SendStall);   
				}
			}
		}
	}
	
}





/*------------------------ concrete handler functions ------------------------------*/


/***************************************************************************/
/*		USB Standard Device Request         											*/
/***************************************************************************/
void (* const edict_StandardDeviceRequest[])(void) = {
	get_status,
	clear_feature,
	reserved,
	set_feature,
	reserved,
	set_address,
	get_descriptor,
	reserved,
	get_configuration,
	set_configuration,
	get_interface,
	set_interface,
	reserved,
	reserved,
	reserved,
	reserved
};

/*
void (* const edict_VendorDeviceRequest[])(void) = {
	TPEDict_ClassRequestHandler
};
*/

void (* const ms_ClassDeviceRequest[])(void) = {
	TPBulk_ClassRequestHandler
};


/******************************** usb request handler ***************************/
void edict_control_handler(void)
{
	unsigned char type, req;

	type = ControlData.DeviceRequest.bmRequestType & USB_REQUEST_TYPE_MASK;
	req  = ControlData.DeviceRequest.bRequest & USB_REQUEST_MASK;

    //	printf ("edict_control_handler type=0x%02x, req=0x%02x\n", type, req);

	if (type == USB_STANDARD_REQUEST)
		(*edict_StandardDeviceRequest[req])();
	else if ( type == USB_CLASS_REQUEST)
		(*ms_ClassDeviceRequest[0])();
	else
	{
        setindex(0);
		set_shortreg(R_USB_TxCSR, CSR0_SendStall); 
	}
}


void usb_EPx_clear_fifo (int ep)
{
	// single fifo
   
	setindex (ep);
	// clear Tx FIFO
    set_shortreg(R_USB_TxCSR, TXCSR_TxPktRdy);
    set_shortreg(R_USB_TxCSR, TXCSR_FlushFIFO);
	
	// clear Rx FIFO
	if(check_u16reg_assert(*R_USB_RxCSR, RXCSR_RxPktRdy))
		set_shortreg(R_USB_RxCSR,RXCSR_FlushFIFO);
}
/***************************************************************************/
/* USB Protocol Layer defined in CHAP.9 of USB1.1 SPEC							*/
/***************************************************************************/
void reserved(void)
{
	usb_StallEP0();
}

/***************************************************************************/
/* USB standard device requests															*/
/***************************************************************************/
void get_status(void)
{
	unsigned char txdat[2];
	unsigned char bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;

	printf ("get_status\n");

	if (bRecipient == USB_RECIPIENT_DEVICE)
	{
		txdat[0] = 1;
		txdat[1] = 0;
		usb_EP0_transmit (txdat, 2);
	}
	else if (bRecipient == USB_RECIPIENT_INTERFACE)
	{
		txdat[0] = 0;
		txdat[1] = 0;
		usb_EP0_transmit (txdat, 2);
	}
	else if (bRecipient == USB_RECIPIENT_ENDPOINT)
	{
		unsigned char endp, c;
		c = 0;
		endp = (unsigned char)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);
   

		if(c & 0x01)
			txdat[0] = 1;
		else
			txdat[0] = 0;
		txdat[1] = 0;
		usb_EP0_transmit (txdat, 2);
	}
	else
	{
		usb_StallEP0();
	}
}

void clear_feature(void)
{
	unsigned char bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;

	printf ("clear_feature bRecipient=%d\n", bRecipient);

	if (bRecipient == USB_RECIPIENT_ENDPOINT \
		&& ControlData.DeviceRequest.wValue == USB_FEATURE_ENDPOINT_STALL)
	{
		uchar endp, direct;
		endp = (unsigned char)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);
        //printf("endp = %d\n",endp);
        direct = (unsigned char)(ControlData.DeviceRequest.wIndex & USB_ENDPOINT_DIRECTION_MASK);
		if(direct == USB_RECEIVE_INDEX)
		{
   
			TPBulk_ClearBulkOutEP(endp);
			
		}
        else if(direct == USB_TRANSMIT_INDEX)
            TPBulk_ClearBulkInEP(endp);

		else
		{
			usb_StallEP0 ();
			
		}

	   
	}
	else
	{
		usb_StallEP0();
	}


}

void set_feature(void)
{
	unsigned char bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;

	printf ("set_feature\n");

	if (bRecipient == USB_RECIPIENT_ENDPOINT
		&& ControlData.DeviceRequest.wValue == USB_FEATURE_ENDPOINT_STALL)
	{
		usb_EP0_transmit (0, 0);
	}
	else
	{
		usb_StallEP0();
	}
}


void set_address(void)
{
	u8_t address;	


	address = (unsigned char)(ControlData.DeviceRequest.wValue & DEVICE_ADDRESS_MASK);
    //	printf ("set_address %d\n", address);

	setindex (0);

    set_shortreg(R_USB_TxCSR, CSR0_ServicedRxPktRdy);
    set_shortreg(R_USB_TxCSR, CSR0_DataEnd);

    // wait until host receive a null packages from the former address
    while(check_u16reg_deassert(*R_USB_IntrTx, INTRTXE_EP0)); 


	// then we can set address of device
    outx(address, R_USB_FAddr);

}

void get_string_descriptor(void)
{
	uchar bIndex = LSB (ControlData.DeviceRequest.wValue);
	u16_t	wLength = ControlData.DeviceRequest.wLength;

    //	printf ("get_string_descriptor bIndex=%d, wLength=%d\n", bIndex, wLength);

	if(bIndex == 0)	// get language ID
	{
		usb_EP0_transmit ((uchar *)&LangDescr, wLength);	//sizeof(USB_LANGID_DESCRIPTOR));
	}
	else if(bIndex == 1) // get Manufacturer
	{
		usb_EP0_transmit ((uchar *)&ms_ManuDescr[0], wLength);	// ms_ManuDescr[0]);
	}
	else
		usb_StallEP0 ();
}

void get_descriptor(void)
{
	unsigned char bDescriptor = MSB(ControlData.DeviceRequest.wValue);
    //#if USB_DEBUG
    //    printf("protocol = %d\n",UsbFlags.protocol);
    //#endif
	if(UsbFlags.protocol == USB_PROTOCOL_FML)
	{
		if (bDescriptor == USB_DEVICE_DESCRIPTOR_TYPE)
		{
			usb_EP0_transmit ((uchar *)&fml_device_descriptor, fml_device_descriptor.bLength);
		}
		else if (bDescriptor == USB_CONFIGURATION_DESCRIPTOR_TYPE)
		{
			u32_t len = 0;

			// makeup configuration descriptor
			memcpy (ControlData.DataBuffer, &fml_configuration_descriptor, fml_configuration_descriptor.bLength);
			len = fml_configuration_descriptor.bLength;

			// copy FML Protocol
			memcpy (ControlData.DataBuffer + len, &fml_interface_descriptor, fml_interface_descriptor.bLength);
			len += fml_interface_descriptor.bLength;
			memcpy (ControlData.DataBuffer + len, &endpoint_b_descriptor, endpoint_b_descriptor.bLength);
			len += endpoint_b_descriptor.bLength;
			memcpy (ControlData.DataBuffer + len, &endpoint_c_descriptor, endpoint_c_descriptor.bLength);
			len += endpoint_c_descriptor.bLength;

			usb_EP0_transmit (ControlData.DataBuffer, (unsigned short)len);
		}
		else if (bDescriptor ==  USB_STRING_DESCRIPTOR_TYPE)
		{
			get_string_descriptor ();
		}
		else
		{
			usb_StallEP0();
		}
	}
	else if(UsbFlags.protocol == USB_PROTOCOL_MS)
	{
		if (bDescriptor == USB_DEVICE_DESCRIPTOR_TYPE)
		{
            
			usb_EP0_transmit ((uchar *)&ms_device_descriptor, ms_device_descriptor.bLength);
		}
		else if (bDescriptor == USB_CONFIGURATION_DESCRIPTOR_TYPE)
		{
			u32_t len = 0;

		//	printf ("USB_CONFIGURATION_DESCRIPTOR_TYPE\n");

			// makeup configuration descriptor
			memcpy (ControlData.DataBuffer, &ms_configuration_descriptor, ms_configuration_descriptor.bLength);
			len = ms_configuration_descriptor.bLength;

			// copy FML Protocol
			memcpy (ControlData.DataBuffer + len, &ms_interface_descriptor, ms_interface_descriptor.bLength);
			len += ms_interface_descriptor.bLength;
			memcpy (ControlData.DataBuffer + len, &endpoint_a_descriptor, endpoint_a_descriptor.bLength);
			len += endpoint_a_descriptor.bLength;
			memcpy (ControlData.DataBuffer + len, &endpoint_d_descriptor, endpoint_d_descriptor.bLength);
			len += endpoint_d_descriptor.bLength;

			usb_EP0_transmit (ControlData.DataBuffer, (unsigned short)len);
		}
		else if (bDescriptor ==  USB_STRING_DESCRIPTOR_TYPE)
		{
		//	printf ("USB_STRING_DESCRIPTOR_TYPE\n");
			get_string_descriptor ();
		}
		else
		{
			usb_StallEP0();
		}
	}

	
}



/************************ config and unconfig related endpoints **************************/
// Configure USB device (Active EPa(IN) & EPd(OUT) )


void ms_usb_config (void)
{
	
	// Config tx Endpoint(transmit)
	// EndPoint Number FROM IOCTRL, Direction (device to host)

	setindex (BULK_EP_NUM);

	outx(BULK_PACKET_SIZE, R_USB_TxMaxP);
	// config EPtx's FIFO
    #if     CONFIG_DBUFFER    
	outx (0x10 | CONFIG_PACKET_SIZE, R_USB_TxFIFOsz);	//  single-packet buffering
    #else
    outx(CONFIG_PACKET_SIZE, R_USB_TxFIFOsz);
    #endif
    
    outx (BULKTX_EP_FIFO_ADDRESS >> 3, R_USB_TxFIFOadd);        // double-buffering
	// EPtx reset
    set_shortreg(R_USB_TxCSR, TXCSR_ClrDataTog);


    // Config tx Endpoint(receive)
	// EndPoint Number FROM IOCTRL, Direction (host to device)

	outw (BULK_PACKET_SIZE, R_USB_RxMaxP);
	// config EPrx's FIFO
    #if     CONFIG_DBUFFER
	outx (0x01 | CONFIG_PACKET_SIZE, R_USB_RxFIFOsz);	//  single-packet buffering
    #else
    outx (CONFIG_PACKET_SIZE, R_USB_RxFIFOsz);        // double-buffering
    #endif
    
    outx (BULKRX_EP_FIFO_ADDRESS >> 3, R_USB_RxFIFOadd);
	// EPrx reset
	set_shortreg(R_USB_RxCSR, RXCSR_ClrDataTog);


    /* set dma control register : burst mode 1, ep num bulk_ep_num */ // ##
    outx(BULK_EP_NUM << CNTL_EpNum | CNTL_BurstMode1 << CNTL_BurstMode, R_USB_DmaCtrl);
    

	UsbFlags.command_state = USBFSM_IDLE;
	UsbFlags.configuration = 1;
    usb_dma_status = DMA_USB_IDLE;
    /* here fill csw structure */
    TPBulk_CommandStatus.bCSW_Status = 0;
    TPBulk_CommandStatus.dCSW_Signature = MS_CSW_SIGNATURE;

    
}

void ms_usb_unconfig (void)
{
  
    clr_shortreg(R_USB_IntrTxE, BULK_EP_NUM);
    clr_shortreg(R_USB_IntrRxE, BULK_EP_NUM);

	UsbFlags.command_state = USBFSM_IDLE;
    usb_dma_status = DMA_USB_IDLE;
	UsbFlags.configuration = 0;

}

void usb_config (void)
{
	return ms_usb_config ();
}

void usb_unconfig (void)
{  
    return ms_usb_unconfig ();
}

/********************************* config functions' tail ***********************/


void set_configuration(void)
{
    //	printf ("set_configuration %d\n", ControlData.DeviceRequest.wValue);

	if (ControlData.DeviceRequest.wValue == 0)
	{
	//	printf ("usb_unconfig\n");
		usb_unconfig();
		usb_EP0_transmit (0, 0);
	}
	else if (ControlData.DeviceRequest.wValue == 1)
	{
	//	printf ("usb_config\n");
		usb_config();
		usb_EP0_transmit (0, 0);
	}
	else
	{
		usb_StallEP0();
	}
}

void get_configuration(void)
{
	unsigned char c; 
	c = UsbFlags.configuration;
	
//	printf ("get_configuration %d\n", c);

	usb_EP0_transmit (&c, 1);
}

void set_interface(void)
{
//	printf ("set_interface\n");

	if (	ControlData.DeviceRequest.wValue == 0
		&& ControlData.DeviceRequest.wIndex == 0)
	{
		// do nothing
		usb_EP0_transmit (0, 0);
	}
	else
	{
		usb_StallEP0();
	}
}

void get_interface(void)
{
	// only one interface (0) support
	unsigned char txdat;

//	printf ("get_interface\n");

	txdat = 0; 
	usb_EP0_transmit (&txdat, 1);
}

/*------------------------------------------concrete handler tail---------------------------------------*/

/*********** bulk tx function *********************/

unsigned long usb_bulk_loadfifo(unsigned char *buf,unsigned long le,unsigned short ep_num)
{
    unsigned int i;
	u8* fifoaddr;
    setindex(ep_num);
   // puts("enter loadfifo\n");
   // printf("ep_num = %d\n",ep_num);
	fifoaddr = (u8*)(R_USB_FIFO0 + (ep_num)*4) ;

	if(le >= BULK_PACKET_SIZE)
	{
       // puts("enter too\n");
	   for(i=0; i<BULK_PACKET_SIZE; i++){
           //puts("enter too\n");
           outx(*(buf + i),fifoaddr);
	   }
	  
	   le = BULK_PACKET_SIZE;
	  // set_shortreg(inw(USB_EP2_TXCSR),0);
	}
	else
	{
	   for(i=0; i<le;i++ ) {
           outx(*(buf + i),fifoaddr);
	   }
	  
	}
	
	return le;
}


void ep_bulk_tx(void)
{
    #ifdef USB_DEBUG
       // puts(".");
    #endif
   //printf("states is %d\n",bulk_only_in_now.trans_status);
  // puts("enter ep1_tx\n");
    setindex(BULK_EP_NUM);
    if(check_u16reg_assert(*R_USB_TxCSR, TXCSR_SentStall)){
    
        clr_shortreg(R_USB_TxCSR, TXCSR_SentStall);    // clr stall
       
    }
    service_transmit();
    if(bulk_only_in_now.trans_status != ALL_TRANS_OVER){
              
        set_shortreg(R_USB_TxCSR, TXCSR_TxPktRdy);
    }
    
  
}


int service_transmit(void)
{
    int len;
    switch(bulk_only_in_now.trans_status)
    {
    case TRANS_ING:
           // service_transmit_buffer();
            #ifdef USB_DEBUG
          // puts(".");
           #endif
        len = usb_bulk_loadfifo(bulk_only_in_now.buffer_addr, bulk_only_in_now.data_left_len, BULK_EP_NUM);
        bulk_only_in_now.buffer_addr += len;
        //bulk_only_in_now.offset_now  += len;
        bulk_only_in_now.data_already_trans +=len;
        bulk_only_in_now.data_left_len -=len;
        if(bulk_only_in_now.data_left_len == 0)
        bulk_only_in_now.trans_status = TRANS_OVER;//TRANS_EMPTY_PACK;
        break;
    //case TRANS_EMPTY_PACK:
     //   bulk_only_in_now.trans_status = TRANS_OVER;
      //  break;
    case TRANS_OVER:
        #ifdef USB_DEBUG
           // puts("trans bsw\n");
        #endif
        TPBulk_ErrorHandler (CASEOK, TPBulk_CommandBlock.dCBW_DataXferLen);
        usb_bulk_loadfifo((u8*)&TPBulk_CommandStatus, sizeof(TPBulk_CommandStatus), BULK_EP_NUM);
        bulk_only_in_now.trans_status = TRANS_BSW_OVER;
        
        break;
    default:
        {
            UsbFlags.command_state = USBFSM_IDLE;
            bulk_only_in_now.trans_status = ALL_TRANS_OVER;
            break;
        }
         
    }
    
        return 0;
        
}    

/**************************** bulk tx function end *****************************/


/**************************** bulk rx function **********************************/

unsigned long usb_bulk_unloadfifo(unsigned char *buf,unsigned long le,unsigned short ep_num){  
	unsigned int i,j;
    volatile int * intptr;
	u32* fifoaddr;
    setindex(ep_num);
	fifoaddr = (u32*)((u32)R_USB_FIFO0 + (ep_num)*4) ;;
  //   printf("fifoaddr = 0x%x\n",fifoaddr);
    
    j = 0;
    #if CONFIG_DBUFFER
    intptr = (volatile int *)buf;
	if(le >=  2*BULK_PACKET_SIZE )	{		// double buffering
    for(;j<2;j++){
        for(i = 0; i < (BULK_PACKET_SIZE >> 2); i++)  {   
         *(intptr++) = inl(fifoaddr);
          
	   }  
        set_shortreg(R_USB_RxCSR, RXCSR_FlushFIFO);
    }
	    
      
	   le = 2*BULK_PACKET_SIZE ;
       
       
        //clr_shortreg(*(vu16 *)usb_mentor.R_RxCSR,0);
	   
	}
   else if(le > BULK_PACKET_SIZE && le< 2*BULK_PACKET_SIZE){ 
       for(i = 0;i < (BULK_PACKET_SIZE >> 2);i++){
           *(intptr++)=inl(fifoaddr); 
          // printf("less_%d %x\n",i,*(intptr-1));
       }
       set_shortreg(R_USB_RxCSR, RXCSR_FlushFIFO);
           for(i = 0;i < le - BULK_PACKET_SIZE;i++){
           *(buf+i)=inb(fifoaddr); 
          // printf("less_%d %x\n",i,*(a+i));

       } 
           set_shortreg(R_USB_RxCSR, RXCSR_FlushFIFO);
           
       }
       
       
        	//double buffering ,so need to set flushfifo twice
  
      
    else{
        for(i=0;i<le;i++){
            *(buf+i)=inb(fifoaddr); 
           // printf("data_%x = %x\n",i,*(a+i));
        } 
       // bulk_rec_para.status.bits.USB_SERVICE_DBUFFER = BUFFER_FIRST;
       set_shortreg(R_USB_RxCSR, RXCSR_FlushFIFO);	//double buffering ,so need to set flushfifo twice

        //clr_shortreg(*(vu16 *)usb_mentor.R_RxCSR,0);
    }

    #else
    	if(le >= BULK_PACKET_SIZE)
	{
    
	   for(i=0;i<EPa_PACKET_SIZE;i++){
           
           *(buf+i) = inb(fifoaddr);
	   }
	  
	   le=EPa_PACKET_SIZE;
       clr_shortreg(R_USB_RxCSR, RXCSR_RxPktRdy);
    }
    else{
        for(i = 0;i < le;i++)
            *(buf+i) = inb(fifoaddr);
        clr_shortreg(R_USB_RxCSR, RXCSR_RxPktRdy);
    }
    
    #endif
    return le;
}



#ifndef USB_DMA_MODE    // here we don't use it

int service_receive()
{
    int len, num;
    if(check_u16reg_assert(*R_USB_RxCSR, RXCSR_RxPktRdy)){
         if(bulk_only_out_now.trans_status == TRANS_ING)

         {
             num = inx(R_USB_RxCount);
         
             #ifdef USB_DEBUG
               // puts(".");
                // printf("num = 0x%x\n",num);
                //printf("0x%x ",bulk_only_out_now.data_left_len);
             #endif
             len = usb_bulk_unloadfifo(bulk_only_out_now.buffer_addr, bulk_only_out_now.data_left_len, BULK_EP_NUM);
             bulk_only_out_now.buffer_addr += len;
             //bulk_only_out_now.offset_now  += len;
             bulk_only_out_now.data_already_got +=len;
             bulk_only_out_now.data_left_len -=len;
            // service_receive_buffer();
            //if( bulk_only_out_now.data_left_len < 0x80 )
              //  printf("data left = 0x%x\n",bulk_only_out_now.data_left_len);
             if( bulk_only_out_now.data_left_len == 0){
                 #ifdef USB_DEBUG
                // puts("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                 puts("#");
                 #endif
                 bulk_only_out_now.trans_status = TRANS_OVER; 


                 TPBulk_ErrorHandler (CASEOK, TPBulk_CommandBlock.dCBW_DataXferLen);
                 bulk_only_in_now.trans_status = ALL_TRANS_OVER;
                 UsbFlags.command_state = USBFSM_CSWPROC;
                 usb_bulk_loadfifo(&TPBulk_CommandStatus, sizeof(TPBulk_CommandStatus), BULK_EP_NUM);
                 setindex(BULK_EP_NUM);
                 set_shortreg(R_USB_TxCSR, TXCSR_TxPktRdy);
                 
             }
             
              
         }

    }
   
    
    return 0;
        
}

#endif

void ep_bulk_rx()
{
    
	int ret;
	unsigned short fifo_remain = 0x0;

	setindex (BULK_EP_NUM);
    //puts("*");
   //printf ("R_RxCSR=0x%04x\n", *(vu16 *)usb_mentor.R_RxCSR);
     
    

   /* if(fifo_remain == 0 && usb_dma_status == DMA_USB_WAITEMPTY)
    {
         clr_shortreg(*(vu16 *)usb_mentor.R_RxCSR,0);
         puts("enter csw\n");

        TPBulk_ErrorHandler (CASEOK, TPBulk_CommandBlock.dCBW_DataXferLen);
        bulk_only_in_now.trans_status = ALL_TRANS_OVER;
        UsbFlags.command_state = USBFSM_CSWPROC;
        usb_dma_status = DMA_USB_IDLE;
        usb_bulk_loadfifo(&TPBulk_CommandStatus, sizeof(TPBulk_CommandStatus), 1);
        setindex(1);
        set_shortreg(*(vu16 *)usb_mentor.R_TxCSR,0);
        
    }

          */    
           //  puts(".");                                        
   if(check_u16reg_assert(*R_USB_RxCSR, RXCSR_SentStall))
   {
   
   	// sentstall,so clear the bit
       clr_shortreg(R_USB_RxCSR, RXCSR_SentStall);
       puts("%");
     
   }

 //fifo_remain = get_Rx_fifo_count(usb_ctrl);
 
   
 //printf ("fifo_remain=%d\n", fifo_remain);

   // #ifdef USB_DMA_MODE
   // if(fifo_remain == 0 && usb_dma_status == DMA_USB_WAITEMPTY)
   //     clr_shortreg(*(vu16 *)usb_mentor.R_RxCSR,0);
   // #endif
    
   // printf ("usb_EPd_rxdone state=%d\n", UsbFlags.command_state);
   // printf ("size of dma left = %d\n", *(vu32 *)USB_DMA_BYTE_COUNT);
   /*
   if(usb_dma_status == DMA_USB_SINGLERX)
    {
        enable_interrupts();
        disable_irq(INT_AS3310_USB_DMA);

                                                    
        fifo_remain = inx(R_USB_RxCount);
                                                  
                                                       
        // printf ("fifo_remain=%d\n", fifo_remain);     

        //puts("get rx\n");
        //printf("dma_ctrl = 0x%x dma_addr = 0x%x dma_count = 0x%x\n",*(vu32 *)USB_DMA_CNTL, \
        //       *(vu32 *)USB_DMA_AHB_ADDR,*(vu32 *)USB_DMA_BYTE_COUNT);
      
        
        usb_dma_status = DMA_USB_RX;
        inx(R_USB_DmaAddr)  = bulk_only_out_now.buffer_addr;
        inx(R_USB_DmaCount) = bulk_only_out_now.data_left_len;
        //inx(R_USB_DmaCtrl) |= (BULK_EP_NUM << CNTL_EpNum);
        
        inx(R_USB_DmaCtrl) |= (1 << CNTL_INTRE);
        clr_intreg(R_USB_DmaCtrl , CNTL_DIC);   // rx endpoint
        clr_intreg(R_USB_DmaCtrl , CNTL_DMAMode);   // dma mode 0
        //inx(R_USB_DmaCtrl) |= (CNTL_BurstMode1 << CNTL_BurstMode);   // burst mode 1
        set_intreg(R_USB_DmaCtrl , CNTL_DMAEN);     // just enable usb dma
        usb_dma_status = DMA_USB_RX;
    }
    */
    #ifndef USB_DMA_MODE
    if(UsbFlags.command_state == USBFSM_DATAOUT){
        service_receive();
    }

    #endif
    
    else if(UsbFlags.command_state == USBFSM_IDLE)
	{
		// check FIFO empty or not
	    if(check_u16reg_assert(*R_USB_RxCSR, RXCSR_RxPktRdy)) {
	    	fifo_remain = inx(R_USB_RxCount);
	}
		
	   // printf ("fifo_remain2=%d\n", fifo_remain);

		if (fifo_remain)
		{   
           
            ret = TPBulk_CBWHandler ();
            #ifdef USB_DEBUG
		   // printf ("TPBulk_CBWHandler end %d\n", ret);
            #endif
            
		}
		else
		{
			/*
			// Zero ACK Package
			usb_ctrl->R_RxCSR &= ~RXCSR_RxPktRdy;
			set_shortreg (usb_ctrl->R_RxCSR, 4);
			usb_ctrl->R_RxCSR |= RXCSR_ClrDataTog;
			*/
		}
	}
    

}

/**************************** bulk rx function end ******************************/

////////////////////////////////////////////////////////////////////////
//      usb mc interupt handler function                              //
// /////////////////////////////////////////////////////////////////////
int do_irq_usb(int priv)
{


	unsigned char   int_value ;
	unsigned char   DEVCTL_reg;	
	unsigned short  INTRTX_reg;
	unsigned short  INTRRX_reg;

	//putc('.');

	//disable_interrupts();
    //
	//// Indicate inside ISR 
	//disable_irq(INT_AS3310_USB_CTRL); 0116
    
	UsbFlags.in_isr = 1;

    DEVCTL_reg  = inx(R_USB_DevCtl);
	int_value   = inx(R_USB_IntrUSB);

    INTRTX_reg = inx(R_USB_IntrTx);
    INTRRX_reg = inx(R_USB_IntrRx);


    //maybe intr events happen in the same time
	if ( check_u8reg_assert(int_value,INTRUSB_Resume)){
		//resume routine;  
        clr_charreg(R_USB_Power, POWERUSB_Resume);
        puts("resume\n");          
	}
	
	if( check_u8reg_assert(int_value, INTRUSB_SessReq)){
		if ( check_u8reg_deassert(DEVCTL_reg, DEVCTRLUSB_BDevice) \
              && check_u8reg_assert(DEVCTL_reg,DEVCTRLUSB_Session)){
			//session req routine
            puts("session req\n");			
		}	
	}
	
	if( check_u8reg_assert(int_value,INTRUSB_VBusError) )
	{
		if (check_u8reg_deassert(DEVCTL_reg,DEVCTRLUSB_BDevice) \
            && check_u8reg_assert(DEVCTL_reg,DEVCTRLUSB_Session)){
			//vbus error
            puts("vbus error\n");
	   
		}
	}

	if( check_u8reg_assert(int_value,INTRUSB_Suspend))
	{
		if (check_u8reg_deassert(DEVCTL_reg,DEVCTRLUSB_HostMode))	{
			//suspend routine
   //       puts("suspend\n");			
      		
		}
	}
	
	if( check_u8reg_assert(int_value,INTRUSB_Conn))
	{
		if( check_u8reg_assert(DEVCTL_reg,DEVCTRLUSB_HostMode)){
			//connect routine
            puts("connect\n");
		}
	}

	if( check_u8reg_assert(int_value,INTRUSB_Discon))
	{
		//disconnect routine
        
        //Soft Disconnect for re-emulate to High Speed
        outx(0x00,R_USB_Power);
        delay(0x10000);
        puts("disconnect\n");

    #if CONFIG_USB_HIGH_SPEED
        //enable Soft Conn and High Speed
	    outb(0x60,R_USB_Power);
    #else
        //enable Soft Conn and Full Speed
        outb(0x40,R_USB_Power);
    #endif
	}
	
	if ( check_u8reg_assert(int_value,INTRUSB_Reset))
	{
		if( check_u8reg_assert(DEVCTL_reg,DEVCTRLUSB_HostMode))
		{
			//babble routine
			puts("babble\n");
		}
		else
		{
			// reset routine
		   
			 UsbFlags.setup_packet = 0;
			 UsbFlags.control_state = 0;
			 UsbFlags.command_state = 0;
			 UsbFlags.configuration = 0;
             usb_dma_status = DMA_USB_IDLE;

 //        puts("reset\n");
			//	bus_reset();
		}
	}

	if( check_u8reg_assert(int_value,INTRUSB_SOF)){
		//sof routine;
      //  puts("sof\n");
	}

  
    
    //printf ("INTRTX_reg=0x%04x\n", INTRTX_reg);
    //printf ("INTRRX_reg=0x%04x\n", INTRRX_reg);

	if (check_u16reg_assert(INTRTX_reg,INTRTXE_EP0))      
	{
//		printf ("usb_EP0_setup\n");
		usb_EP0_setup();           //ep0 tx routine
	}

 if (check_u16reg_assert(INTRTX_reg,INTRTXE_EP1_Tx))   // ###here we can set as private data   
     {
        //puts("ep1 tx\n");
        ep_bulk_tx();   
     }                 //ep1 tx routine


	if (check_u16reg_assert(INTRRX_reg, INTRRXE_EP1_Rx))                	{
		//printf ("EPd_rxdone\n");
		ep_bulk_rx ();
	}



	outl (USB_NC_ICOLL_CLEAR, INT_IRQCLEAR1 + 4); // set Clear

	
	if(UsbFlags.setup_packet)
	{
		UsbFlags.setup_packet = 0;
		edict_control_handler();
	}

	UsbFlags.in_isr = 0;

    //enable_irq(INT_AS3310_USB_CTRL);
    //
    //if(usb_dma_status != DMA_USB_IDLE && usb_dma_status != DMA_USB_SINGLERX)
    //    enable_irq(INT_AS3310_USB_DMA);
	//enable_interrupts();      0116

    return 0;

}

////////////////////////////////////////////////////////////////////////
//      usb dma interupt handler function                              //
// /////////////////////////////////////////////////////////////////////

int do_irq_usb_dma(int priv)
{   
    if(check_u16reg_deassert(*R_USB_DmaIntr, 0))
    return -1;  // must check it , for only when intr set the data is 
                // completly transmited or received 
  
    
    setindex(BULK_EP_NUM);
    //disable_interrupts();
    //disable_irq(INT_AS3310_USB_DMA);
    //disable_irq(INT_AS3310_USB_CTRL); 0116

   
    #ifdef USB_DEBUG
         //puts("enter dma intr\n");
    #endif
   

    if(usb_dma_status == DMA_USB_RX){
        #ifdef USB_DEBUG
           puts("dma rx\n");
        #endif
        //printf("rxcsr = %x\n",*(vu16 *)usb_mentor.R_RxCSR);
       // printf("data left size:0x%x\n",*(vu16 *)usb_mentor.R_RxCount);

            /*
            if(check_u16reg_assert(*(vu16 *)usb_mentor.R_RxCSR, 0))
        {
          //  puts("should clr rxpkgrdy\n");
          //  clr_shortreg(*(vu16 *)usb_mentor.R_RxCSR,0);
            //set_shortreg(*(vu16 *)usb_mentor.R_RxCSR,4);
            //set_shortreg(*(vu16 *)usb_mentor.R_RxCSR,4);
        }
            */
       // printf("count left = %x\n",*(vu32 *)usb_mentor.R_DmaCount);
       // printf("last_got = 0x%x offset = %d\n",bulk_only_out_now.data_last_got,bulk_only_out_now.offset_now);
        
            //usb_mem_dev->ioctl( usb_mem_dev, DEV_MEM_WRITE_BUFFER_OK, 0);
            usb_mem_dev->write(usb_mem_dev, bulk_only_out_now.offset_now, mass_buf, 1);
            if(bulk_only_out_now.data_left_len >0){

            
           // puts(".");
                //enable_interrupts();
                //enable_irq(INT_AS3310_USB_DMA);   0116
        
            if(bulk_only_out_now.data_left_len >= buffer_size)
               {
                // puts("&");
                    inx(R_USB_DmaCount) = buffer_size;
                    bulk_only_out_now.data_left_len -= buffer_size;
                 
                    bulk_only_out_now.offset_now += (bulk_only_out_now.data_last_got >> 9);
                   /// while((mass_buf = usb_mem_dev->mmap(usb_mem_dev, 0, bulk_only_out_now.offset_now)) == NULL);
               
                    bulk_only_out_now.data_last_got = buffer_size;
               }   
             else //if(bulk_only_out_now.data_left_len > 0)
                {
                      inx(R_USB_DmaCount) = bulk_only_out_now.data_left_len;
                      
                      bulk_only_out_now.offset_now += (bulk_only_out_now.data_last_got >> 9);
                      bulk_only_out_now.data_last_got = bulk_only_out_now.data_left_len;
                      bulk_only_out_now.data_left_len = 0;
                      
                }
             inx(R_USB_DmaAddr) = (u32)mass_buf;
             
             inx(R_USB_DmaCtrl) &= 0xff0;  // clear dma ctrl reg except for burst mode and ep num
                                      // see usb.c ms_usb_config(): 
                                      // /* set dma control register : burst mode 1, ep num bulk_ep_num */
        
             inx(R_USB_DmaCtrl) |= 0xc;
        
             clr_intreg(R_USB_DmaCtrl , CNTL_DIC);
        
             set_intreg(R_USB_DmaCtrl , CNTL_DMAEN);
             #ifdef     USB_DEBUG
           //   printf("dma_ctrl = 0x%x dma_addr = 0x%x dma_count = 0x%x\n",*R_USB_DmaCtrl,
           //  *R_USB_DmaAddr,*R_USB_DmaCount);
             #endif  
             return 0;
        }

     else{
            


             clr_shortreg(R_USB_RxCSR, RXCSR_RxPktRdy); // should clr 
             
             clr_shortreg(R_USB_RxCSR, RXCSR_AutoClear);    // disable auto clear
             clr_shortreg(R_USB_RxCSR, RXCSR_DMAReqEnab);   // disable dma req enable
        
             set_shortreg(R_USB_IntrRxE, BULK_EP_NUM);
       
              //while(check_u16reg_deassert(*R_USB_IntrTx,INTRTXE_EP1_Tx));  // wait for csw req
                       // make sure host can get csw package                                              
              do{
              
             // csw step
              TPBulk_ErrorHandler (CASEOK, TPBulk_CommandBlock.dCBW_DataXferLen);
              bulk_only_in_now.trans_status = ALL_TRANS_OVER;
              UsbFlags.command_state = USBFSM_CSWPROC;
              usb_bulk_loadfifo((u8*)&TPBulk_CommandStatus, sizeof(TPBulk_CommandStatus), BULK_EP_NUM);
             // setindex(BULK_EP_NUM);
              set_shortreg(R_USB_TxCSR, TXCSR_TxPktRdy);
              }
              while(check_u16reg_deassert(*R_USB_IntrTx,INTRTXE_EP1_Tx));
              usb_dma_status = DMA_USB_IDLE;
           
              inx(R_USB_DmaAddr)  = 0;  
              inx(R_USB_DmaCount) = 0; 
              inx(R_USB_DmaCtrl) &= 0xff0; 

             // enable_irq(INT_AS3310_USB_CTRL); // can only enable mac intr after dma trans over
        
         }

    }   // end of rx handler

    else  {
        if(bulk_only_in_now.data_left_len >0){

            usb_mem_dev->read(usb_mem_dev, bulk_only_in_now.offset_now, mass_buf, 1);

            //enable_interrupts();
            //enable_irq(INT_AS3310_USB_DMA);   0116

            //set_shortreg(R_USB_TxCSR, TXCSR_TxPktRdy);

        if(bulk_only_in_now.data_left_len >= buffer_size)
           {
                //puts("%");
                inx(R_USB_DmaCount) = buffer_size;
                bulk_only_in_now.data_left_len -= buffer_size;

                bulk_only_in_now.offset_now += (bulk_only_in_now.data_last_trans >> 9);
               /// while((mass_buf = usb_mem_dev->mmap(usb_mem_dev, 0, bulk_only_out_now.offset_now)) == NULL);

                bulk_only_in_now.data_last_trans = buffer_size;
           }   
         else //if(bulk_only_in_now.data_left_len > 0)
            {
                  inx(R_USB_DmaCount) = bulk_only_in_now.data_left_len;

                  bulk_only_in_now.offset_now += (bulk_only_in_now.data_last_trans >> 9);
                  bulk_only_in_now.data_last_trans = bulk_only_in_now.data_left_len;
                  bulk_only_in_now.data_left_len = 0;

            }
         inx(R_USB_DmaAddr) = (u32)mass_buf;
             
         inx(R_USB_DmaCtrl) &= 0xff0;  // clear dma ctrl reg except for burst mode and ep num
                                  // see usb.c ms_usb_config(): 
                                  // /* set dma control register : burst mode 1, ep num bulk_ep_num */
        
         inx(R_USB_DmaCtrl) |= 0xe;
        
         set_intreg(R_USB_DmaCtrl , CNTL_DMAEN);
         
         #ifdef     USB_DEBUG
          //  printf("dma_ctrl = 0x%x dma_addr = 0x%x dma_count = 0x%x\n",*R_USB_DmaCtrl,
          //   *R_USB_DmaAddr,*R_USB_DmaCount);
         #endif  
        return 0;
        }

        else{
        
       
        clr_shortreg(R_USB_TxCSR, TXCSR_AutoSet);  // disable autoset
        clr_shortreg(R_USB_TxCSR, TXCSR_DMAReqEnab);  // disable dma req
        clr_shortreg(R_USB_TxCSR, TXCSR_DMAReqMode);
        set_shortreg(R_USB_IntrTxE, BULK_EP_NUM);
       // puts("dma tx\n");
        
        
        
        
        TPBulk_ErrorHandler (CASEOK, TPBulk_CommandBlock.dCBW_DataXferLen);
        UsbFlags.command_state = USBFSM_CSWPROC;
        bulk_only_in_now.trans_status = ALL_TRANS_OVER;
        usb_bulk_loadfifo((u8*)&TPBulk_CommandStatus, sizeof(TPBulk_CommandStatus), BULK_EP_NUM);
        
        set_shortreg(R_USB_TxCSR, TXCSR_TxPktRdy);

        while(check_u16reg_deassert(*R_USB_IntrTx,INTRTXE_EP1_Tx));

    
       
        inx(R_USB_DmaAddr)  = 0;  
        inx(R_USB_DmaCount) = 0; 
        inx(R_USB_DmaCtrl) &= 0xff0; 
 
        usb_dma_status = DMA_USB_IDLE;

        //enable_irq(INT_AS3310_USB_CTRL);// can only enable mac intr after dma trans over
        
        }
    }   // end of tx handler
    
 
        //enable_irq(INT_AS3310_USB_CTRL);
        //enable_interrupts();      0116
        return TRUE;

}

/* usb flags and hardware init */
int usb_init(struct device * dev)
{
    irq_action_t usb_mc_irq, usb_dma_irq;
    int ret,dev_index;

    // print configuration of usb device
    
   
    printf("pkgsz = %d  ", BULK_PACKET_SIZE); 
   // printf("cpu_clk =%d  ", CONFIG_USB_CPU_CLK);  
    #if CONFIG_USB_HS
        puts("HIGH SPEED MODE\n");  
    #else
        puts("FULL SPEED MODE\n");
    #endif
    // open devmem device usb_mem_dev
    
    #if (CONFIG_UDISK_MEM|CONFIG_UDISK_AFTL|CONFIG_UDISK_SD_MMC)
    #else
    #error "No U-Disk device choosed!"
    #endif
    
    #if CONFIG_UDISK_MEM
    usb_mem_dev = device_get("devmem",&dev_index);
    //usb_mem_dev_i = (struct devmem *)usb_mem_dev->priv_data;
    lba_offset = usb_mem_dev->ioctl(usb_mem_dev, DEV_MEM_GETSIZE, 0);
    buf_offset = usb_mem_dev->ioctl(usb_mem_dev, DEV_MEM_GET_SECTOR_SHIFT, 0);
    #endif //CONFIG_UDISK_MEM
    
    #if CONFIG_UDISK_AFTL
    usb_mem_dev = device_get("aftl",&dev_index);
    //usb_mem_dev_i = (struct devmem *)usb_mem_dev->priv_data;
    lba_offset = usb_mem_dev->ioctl(usb_mem_dev, NFTL_IOCTL_GET_SECTOR_NUM, 0);
    buf_offset = usb_mem_dev->ioctl(usb_mem_dev, NFTL_IOCTL_GET_SECTOR_SHIFT, 0);
    #endif //CONFIG_UDISK_AFTL

    #if CONFIG_UDISK_SD_MMC
    usb_mem_dev = device_get("sd/mmc",&dev_index);
    //usb_mem_dev_i = (struct devmem *)usb_mem_dev->priv_data;
    lba_offset = usb_mem_dev->ioctl(usb_mem_dev, SD_MMC_IOCTL_GET_SECTOR_NUM, 0);
    buf_offset = usb_mem_dev->ioctl(usb_mem_dev, SD_MMC_IOCTL_GET_SECTOR_SHIFT, 0);
    #endif //CONFIG_UDISK_AFTL
    
    buffer_size = 1 << buf_offset;
    mass_buf = (u8*)mem_usb_buffer;

    #ifdef  USB_DEBUG
        printf("mass_buf = 0x%x buffer_size = 0x%x lba_offset = 0x%x\n",mass_buf, buffer_size, lba_offset);
    #endif

    // disable USB MC interrupt(57)
    disable_irq(INT_AS3310_USB_CTRL);

    // set USB MC interrupt priority
    //outl(0x00000300, HW_ICOLL_PRIORITY14_SET);

	memset (&UsbFlags, 0, sizeof(UsbFlags));

    // USB FLAGS INIT
	UsbFlags.protocol = USB_PROTOCOL_MS;    // select protocol
    usb_dma_status = DMA_USB_IDLE;    

    
    /* usb mc irq register */
    usb_mc_irq.irq = INT_AS3310_USB_CTRL;
    usb_mc_irq.clear = NULL;
    usb_mc_irq.irq_handler = do_irq_usb;
    usb_mc_irq.priv_data = (int)dev;

    ret = request_irq(&usb_mc_irq);
    if (ret) {
        printf("USB MC IRQ %d Error\n",INT_AS3310_USB_CTRL);
    }

    /* usb dma irq register */

    usb_dma_irq.irq = INT_AS3310_USB_DMA;
    usb_dma_irq.clear = NULL;
    usb_dma_irq.irq_handler = do_irq_usb_dma;
    usb_dma_irq.priv_data = (int)dev;

    ret = request_irq(&usb_dma_irq);
    if (ret) {
        printf("USB DMA IRQ %d Error\n",INT_AS3310_USB_DMA);
    }
   
   

    //Soft Disconnect for re-emulate to High Speed
    outx(0x00,R_USB_Power);
    delay(0x10000);
    
     //arm initial, ungate clk and select which phy to use
    outl(0x00000004, USB_CLK + 8); // ungate usb ahb clk

    // usb phy apb config
    #if CONFIG_USB_HIGH_SPEED
        outl(0x7c00, USB_PWD + 8); // clear PWD VBG,V2I,IBIAS,FS,COMP
        
    #else
        outl(0, USB_PWD);
    #endif

    outl(0, USB_TX);
    outl(0, USB_RX);
    outl(0, USB_CTRL);
    outl(0x00004000, USB_SYSCTRL);
    outl(0x11, USB_ANALOG);

    #if CONFIG_USB_HIGH_SPEED
        hs_calibration();
    #endif  //CONFIG_USB_HIGH_SPEED

    // finish Calibration

    /* soft conn */
    otg_init();

    /* enable usb intr */
    usb_interrupt_enable();
	 
	 // enable interrupt service
	enable_irq(INT_AS3310_USB_CTRL);
  
	 
#if CONFIG_USB_HIGH_SPEED
    puts("USB 2.0 High Speed\n");
#else
    puts("USB 1.1 Full Speed\n");
#endif	
    return 0;
}


int usb_probe(struct device * dev){

    usb_init(dev);
    return 0;
}
int usb_ioctl(struct device * dev,unsigned int cmd,unsigned long arg){
    // reserved
    return 0;
}

struct device dev_usb = {
    .name       = "UsbStorage",
    .dev_id     = 0,
    .probe      = usb_probe,
    .remove     = NULL,  
    .open       = NULL,  
    .close      = NULL,  
    .read       = NULL,  
    .write      = NULL, 
    .ioctl      = usb_ioctl, 
    .suspend    = NULL, 
    .resume     = NULL, 
    .status     = 0,
    .priv_data  = NULL, 
};

__add_device_lv1(dev_usb);


