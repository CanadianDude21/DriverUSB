/*
 ============================================================================
 Nom         : driverUSB.c
 Author      : Samuel Pesant et Mathieu Fournier-Desrochers
 Date 	     : 22-10-2019
 Description : Fonctions du pilote série
 ============================================================================
 */

#include "driverUSB.h"


MODULE_AUTHOR("Mathieu Fournier-Desrochers et Samuel Pesant");
MODULE_LICENSE("Dual BSD/GPL");

USBperso *device;

int pilote_USB_probe(struct usb_interface *intf, const struct usb_device_id *id){
	int retval = -ENOMEM;
	struct usb_host_interface *iface_desc;
	struct usb_device *dev = interface_to_usbdev(intf);
	iface_desc = intf->cur_altsetting;

	if(iface_desc->desc.bInterfaceClass == CC_VIDEO && iface_desc->desc.bInterfaceSubClass == SC_VIDEOSTREAMING && iface_desc->desc.bInterfaceNumber == 1){
		device = kmalloc(sizeof(USBperso),GFP_ATOMIC);
		device->myStatus = 0;
		device->myLength = LENGTH;
		device->myLengthUsed = 0;
		device->myData = kmalloc(sizeof(char)*device->myLength,GFP_ATOMIC);
		device->dev = usb_get_dev(dev);
		device->intf = intf;
		usb_set_intfdata(intf,device);
		usb_register_dev(intf, &usbClass);
		usb_set_interface(device->dev, iface_desc->desc.bInterfaceNumber, 4);
		printk(KERN_ALERT"ELE784 -> probe \n\r");
		retval = 0;
	}


	if(iface_desc->desc.bInterfaceClass == CC_VIDEO && iface_desc->desc.bInterfaceSubClass == SC_VIDEOCONTROL && iface_desc->desc.bInterfaceNumber == 0){	
		retval = 0;

	}

	return retval ;
};

static void pilote_USB_disconnect(struct usb_interface *intf){
	
	struct usb_host_interface *iface_desc;
	iface_desc = intf->cur_altsetting;
	if(iface_desc->desc.bInterfaceClass == CC_VIDEO && iface_desc->desc.bInterfaceSubClass == SC_VIDEOSTREAMING && iface_desc->desc.bInterfaceNumber == 1){
		printk(KERN_ALERT "ELE784 -> disconnect \n\r");
		usb_put_dev(device->dev);
		kfree(device->myData);
		kfree(device);
		usb_set_intfdata(intf,NULL);
		usb_deregister_dev(intf,&usbClass);
		
	}

};


int pilote_USB_open(struct inode *inode, struct file *filp){
	
	filp->private_data = device;
	printk(KERN_ALERT "ELE784 -> open \n\r");

	return 0;
};

ssize_t pilote_USB_read(struct file *filp, char *buff, size_t count, loff_t *f_pos){

	USBperso *device = (USBperso*)filp->private_data;

	wait_for_completion(&wait_read);

	printk(KERN_ALERT "ELE784 -> read \n\r");


	return 0;
};

long pilote_USB_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	long retval;
	uint16_t nbPackets;
	int myPacketSize, nbUrbs;
	int i, j;
	size_t size;

	struct urb** myUrb;
	struct usb_endpoint_descriptor endpointDesc;
	struct usb_host_interface *cur_altsetting;
	USBperso *device = (USBperso*)filp->private_data;
	struct usb_interface *interface = device->intf;
 	struct usb_device *udev = device->dev;
	
	
	switch(cmd){
	
	case IOCTL_GET:
		printk(KERN_ALERT"ELE784 -> IOCTL_GET \n\r");
		break;
	case IOCTL_SET:
		printk(KERN_ALERT"ELE784 -> IOCTL_SET \n\r");
		break;
	case IOCTL_STREAMON:
		printk(KERN_ALERT"ELE784 -> IOCTL_STREAMON (0x30) \n\r");
		retval = usb_control_msg(udev,
					usb_sndctrlpipe(udev, 0x00),
					0x0B,
					USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE,
					0x0004, 0x0001,NULL,0x00,0x00);
			
		break;
	case IOCTL_STREAMOFF:
		printk(KERN_ALERT"ELE784 -> IOCTL_STREAMOFF (0x40) \n\r");
		retval = usb_control_msg(udev,
					usb_sndctrlpipe(udev, 0),
					0x0B,
					USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE,
					0x00, 0x01,NULL,0,0);
			
		break;
	case IOCTL_GRAB:
		printk(KERN_ALERT"ELE784 -> IOCTL_GRAB (0x50) \n\r");
		cur_altsetting = interface->cur_altsetting;
		endpointDesc = cur_altsetting->endpoint[0].desc;

		nbPackets = 40;  // The number of isochronous packets this urb should contain			
		myPacketSize = le16_to_cpu(endpointDesc.wMaxPacketSize);			
		size = myPacketSize * nbPackets;
		nbUrbs = 5;

		for (i = 0; i < nbUrbs; ++i) {
		  usb_free_urb(myUrb[i]); // Pour être certain
		  myUrb[i] = usb_alloc_urb(nbPackets,GFP_ATOMIC);
		  if (myUrb[i] == NULL) {
		    //printk(KERN_WARNING "");		
		    return -ENOMEM;
		  }

		  //myUrb[i]->transfer_buffer = usb_buffer_alloc(udev,size,GFP_KERNEL,dma);
		  myUrb[i]->transfer_buffer = usb_alloc_coherent(udev,size,GFP_ATOMIC,&(myUrb[i]->transfer_dma));

		  if (myUrb[i]->transfer_buffer == NULL) {
		    //printk(KERN_WARNING "");		
		    usb_free_urb(myUrb[i]);
		    return -ENOMEM;
		  }

		  myUrb[i]->dev = udev;
		  myUrb[i]->context = udev;
		  myUrb[i]->pipe = usb_rcvisocpipe(udev, endpointDesc.bEndpointAddress);
		  myUrb[i]->transfer_flags = URB_ISO_ASAP | URB_NO_TRANSFER_DMA_MAP;
		  myUrb[i]->interval = endpointDesc.bInterval;
		  myUrb[i]->complete = complete_callback;
		  myUrb[i]->number_of_packets = nbPackets;
		  myUrb[i]->transfer_buffer_length = size;

		  for (j = 0; j < nbPackets; ++j) {
		    myUrb[i]->iso_frame_desc[j].offset = j * myPacketSize;
		    myUrb[i]->iso_frame_desc[j].length = myPacketSize;
		  }								
		}

		for(i = 0; i < nbUrbs; i++){
		  if ((retval = usb_submit_urb(myUrb[i],GFP_ATOMIC)) < 0) {
		    //printk(KERN_WARNING "");		
		    return retval;
		  }
		}
		break;
	case IOCTL_PANTILT:
		printk(KERN_ALERT"ELE784 -> IOCTL_PANTILT \n\r");
		break;
	case IOCTL_PANTILT_RESEST:
		printk(KERN_ALERT"ELE784 -> IOCTL_PANTILT_RESEST \n\r");
		break;
	default:
		return -ENOTTY;
	}
	printk(KERN_ALERT"ELE784 -> ioctl \n\r");
	return retval;
};

static void complete_callback(struct urb *urb){

	int ret;
	int i;
	unsigned char * data;
	unsigned int len;
	unsigned int maxlen;
	unsigned int nbytes;
	void * mem;

	if(urb->status == 0){

		for (i = 0; i < urb->number_of_packets; ++i) {
			if(device->myStatus == 1){
				continue;
			}
			if (urb->iso_frame_desc[i].status < 0) {
				continue;
			}

			data = urb->transfer_buffer + urb->iso_frame_desc[i].offset;
			if(data[1] & (1 << 6)){
				continue;
			}
			len = urb->iso_frame_desc[i].actual_length;
			if (len < 2 || data[0] < 2 || data[0] > len){
				continue;
			}

			len -= data[0];
			maxlen = device->myLength - device->myLengthUsed ;
			mem = device->myData + device->myLengthUsed;
			nbytes = min(len, maxlen);
			memcpy(mem, data + data[0], nbytes);
			device->myLengthUsed += nbytes;

			if (len > maxlen) {
				device->myStatus = 1; // DONE
			}

			/* Mark the buffer as done if the EOF marker is set. */

			if ((data[1] & (1 << 1)) && (device->myLengthUsed != 0)) {
				device->myStatus = 1; // DONE

			}
		}

		if (!(device->myStatus == 1)){
			if ((ret = usb_submit_urb(urb, GFP_ATOMIC)) < 0) {
				//printk(KERN_WARNING "");
			}
		}else{
			complete(&wait_read);
			///////////////////////////////////////////////////////////////////////
			//  Synchronisation
			///////////////////////////////////////////////////////////////////////
		}
	}else{
		//printk(KERN_WARNING "");
	}
}


module_usb_driver(myUSBdriver);
