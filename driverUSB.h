/*
 ============================================================================
 Nom         : driverUSB.h
 Author      : Samuel Pesant et Mathieu Fournier-Desrochers
 Date 	     : 22-10-2019
 Description : Prototypes de fonctions du pilote série
 ============================================================================
 */
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/errno.h> 
#include <linux/syscalls.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/wait.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

 
#include <linux/usb.h>
#include <linux/completion.h>
#include <linux/usb/video.h>
#include "ioctlcmd.h"
#include "usbvideo.h"



#define DEV_MAJOR 250
#define DEV_MINOR 0

#define USB_VENDOR_ID	0x046d 
#define USB_PRODUCT_ID 	0x0994

typedef struct{
	struct usb_device* dev;
}USBperso;

int pilote_USB_probe(struct usb_interface *intf, const struct usb_device_id *id);
static void pilote_USB_disconnect(struct usb_interface *intf);
int pilote_USB_open(struct inode *inode,struct file *filp);
ssize_t pilote_USB_read(struct file *filp, char *buff, size_t count, loff_t *f_pos);
long pilote_USB_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static void complete_callback(struct urb *urb);

struct file_operations monModule_fops = {
	.owner   = THIS_MODULE,
	.open    = pilote_USB_open,
	.read    = pilote_USB_read,
	.unlocked_ioctl   = pilote_USB_ioctl
};

struct usb_class_driver usbClass = {
	.name = "Camera %d",
	.fops = &monModule_fops,
	.minor_base = DEV_MINOR
};

struct usb_device_id MyIdTable[]={
	{USB_DEVICE(USB_VENDOR_ID,USB_PRODUCT_ID)},
	{}
};


struct usb_driver myUSBdriver ={
	.name = "myUSBdriver",
	.probe = pilote_USB_probe,
	.disconnect = pilote_USB_disconnect,
	.id_table = MyIdTable,
};
MODULE_DEVICE_TABLE(usb,MyIdTable);
