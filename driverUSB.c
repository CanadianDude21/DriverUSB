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
	int retval;
	struct usb_host_interface *iface_desc;
	struct usb_device *dev = interface_to_usbdev(intf);

	device = kmalloc(sizeof(USBperso),GFP_KERNEL);
	device->dev = usb_get_dev(dev);
	iface_desc = intf->cur_altsetting;
	if(iface_desc->desc.bInterfaceClass == CC_VIDEO && iface_desc->desc.bInterfaceSubClass == SC_VIDEOSTREAMING && iface_desc->desc.bInterfaceNumber == 1){
		usb_set_intfdata(intf,device);
		usb_register_dev(intf, &usbClass);
		retval = usb_set_interface(device->dev, iface_desc->desc.bInterfaceNumber, 4);
	}
	if(iface_desc->desc.bInterfaceClass == CC_VIDEO && iface_desc->desc.bInterfaceSubClass == SC_VIDEOCONTROL && iface_desc->desc.bInterfaceNumber == 0){
		usb_register_dev(intf, &usbClass);
	}
	printk(KERN_ALERT"ELE784 -> probe \n\r");
	return 0;
};

static void pilote_USB_disconnect(struct usb_interface *intf){

	usb_put_dev(device->dev);
	usb_set_intfdata(intf,NULL);
	usb_deregister_dev(intf,&usbClass);
	printk(KERN_ALERT "ELE784 -> disconnect \n\r");
};


int pilote_USB_open(struct inode *inode, struct file *filp){
	
	struct usb_interface *intf;
	int subminor;
	
	printk(KERN_ALERT "ELE784 -> open \n\r");
	
	subminor = iminor(inode);

	intf = usb_find_interface(&myUSBdriver, subminor);
	if(!intf){
		printk(KERN_WARNING "ELE784 -> open: ne peux ouvrir le peripherique");
		return -ENODEV;	
	}

	filp->private_data = intf;
	return 0;
};

ssize_t pilote_USB_read(struct file *filp, char *buff, size_t count, loff_t *f_pos){
	printk(KERN_ALERT "ELE784 -> read \n\r");
	return 0;
};

long pilote_USB_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	
	switch(cmd){
	
	case IOCTL_GET:
		printk(KERN_ALERT"ELE784 -> IOCTL_GET \n\r");
		break;
	case IOCTL_SET:
		printk(KERN_ALERT"ELE784 -> IOCTL_SET \n\r");
		break;
	case IOCTL_STREAMON:
		printk(KERN_ALERT"ELE784 -> IOCTL_STREAMON \n\r");
		break;
	case IOCTL_STREAMOFF:
		printk(KERN_ALERT"ELE784 -> IOCTL_STREAMOFF \n\r");
		break;
	case IOCTL_GRAB:
		printk(KERN_ALERT"ELE784 -> IOCTL_GRAB \n\r");
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
	return 0;
};

module_usb_driver(myUSBdriver);
