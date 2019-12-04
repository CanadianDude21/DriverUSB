/*
 ============================================================================
 Nom         : driverUSB.c
 Author      : Samuel Pesant et Mathieu Fournier-Desrochers
 Date 	     : 22-10-2019
 Description : Fonctions du pilote usb pour caméra 
 ============================================================================
 */

#include "driverUSB.h"


MODULE_AUTHOR("Mathieu Fournier-Desrochers et Samuel Pesant");
MODULE_LICENSE("Dual BSD/GPL");

USBperso *device; //déclaration de la structure perso
struct urb* myUrb[5]; //déclaration des 5 urb

/* Description: Fonction probe du pilote 
 *
 * Arguments  : usb interface qui fait appel à probe
 *		ID usb_device_id
 *
 * Return     : Code d'erreur ou 0 pour réussite
 */
int pilote_USB_probe(struct usb_interface *intf, const struct usb_device_id *id){
	int retval = -ENOMEM;
	//obtention de l'interface et du usb device
	struct usb_host_interface *iface_desc;
	struct usb_device *dev = interface_to_usbdev(intf);
	iface_desc = intf->cur_altsetting;
	//l'interface qui nous intéresse est le video streaming. 
	if(iface_desc->desc.bInterfaceClass == CC_VIDEO && iface_desc->desc.bInterfaceSubClass == SC_VIDEOSTREAMING && iface_desc->desc.bInterfaceNumber == 1){
		//initialisation des champs de la structure perso 
		device = kmalloc(sizeof(USBperso),GFP_ATOMIC);
		device->myStatus = 0;
		device->myLength = LENGTH;
		device->myLengthUsed = 0;
		device->myData = kmalloc(sizeof(char)*device->myLength,GFP_ATOMIC);
		device->dev = usb_get_dev(dev); // copy de la usb device
		device->intf = intf; // copy de l'interface actuel
		usb_set_intfdata(intf,device); // place la structure de donnée dans l'interface
		usb_register_dev(intf, &usbClass); // s'enregistre pour l'interface à USB
		usb_set_interface(device->dev, iface_desc->desc.bInterfaceNumber, 4); // set l'interface
		printk(KERN_ALERT"ELE784 -> probe \n\r";
		retval = 0; //retourne succèes
	}
	//pour video controle on ne fait rien de spécial
	if(iface_desc->desc.bInterfaceClass == CC_VIDEO && iface_desc->desc.bInterfaceSubClass == SC_VIDEOCONTROL && iface_desc->desc.bInterfaceNumber == 0){	
		usb_register_dev(intf, &usbClass);
		retval = 0; //retourne succèes
	}

	

	return retval ;
};
 //************************************************************************************************************		       

/* Description: Fonction disconnect du pilote 
 *
 * Arguments  : usb interface qui fait appel à disconnect
 *		
 *
 * Return     : void
 */
static void pilote_USB_disconnect(struct usb_interface *intf){
	
	struct usb_host_interface *iface_desc;
	iface_desc = intf->cur_altsetting;
	// vérifie qu'il s'agit de video streaming pour libérer l'espace de ma structure
	if(iface_desc->desc.bInterfaceClass == CC_VIDEO && iface_desc->desc.bInterfaceSubClass == SC_VIDEOSTREAMING && iface_desc->desc.bInterfaceNumber == 1){
		printk(KERN_ALERT "ELE784 -> disconnect \n\r");
		usb_put_dev(device->dev);
		kfree(device->myData);
		kfree(device);
		usb_set_intfdata(intf,NULL);
		usb_deregister_dev(intf,&usbClass); // on se retire 
		
	}
	else 
	{
		usb_register_dev(intf, &usbClass); // AJOUTER PEUT ELENVER LE ELSE SI SA FONCTIONNE PAS
	}
};
 //************************************************************************************************************

		       
/* Description: Fonction d'ouverture du pilote 
 *
 * Arguments  : structure inode
 *		structure file 
 *
 * Return     : void
 */		       
int pilote_USB_open(struct inode *inode, struct file *filp){
	// on place notre structure de donnée dans private data
	filp->private_data = device;
	printk(KERN_ALERT "ELE784 -> open \n\r");
	// aucune vérification n'est demandé dans l'énoncé du lab.
	return 0;
};
//************************************************************************************************************
		       

/* Description: Fonction read qui permet la lecture d'un image 
 *
 * Arguments  : le buffer de destination
 *		le nombre de donné a copier
 *		structure file 
 *		loff_t fpos
 *
 * Return     : retourne le nombre de donné manquante
 */
ssize_t pilote_USB_read(struct file *filp, char *buff, size_t count, loff_t *f_pos){

	int i, ret;
	USBperso *device_perso = (USBperso*)filp->private_data; //CHANGER LE NOM POUR PAS QUI CONFONDRE LES 2 AVANT DEVICE MTN device_perso


	for(i = 0; i<5; i++){
		wait_for_completion(&wait_read);
	}
	ret = (int)copy_to_user(buff,device_perso->myData,count);
	for(i = 0; i<5; i++){
		usb_kill_urb(myUrb[i]);
		usb_free_coherent(device_perso->dev,myUrb[i]->transfer_buffer_length,myUrb[i]->transfer_buffer,myUrb[i]->transfer_dma);
		usb_free_urb(myUrb[i]);
	}

	printk(KERN_ALERT "ELE784 -> read \n\r");


	return (count-ret);
};
//************************************************************************************************************
		       
/* Description: Fonction qui contient toute les commande IOCTL
 *
 * Arguments  :  structure file 
 *		la commande
 *		pointeur sur un argument
 *
 * Return     : retourne la valeur selon la commande
 */		       
long pilote_USB_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	long retval;
	uint16_t nbPackets;
	int myPacketSize, nbUrbs;
	int i, j;
	size_t size;

	struct usb_endpoint_descriptor endpointDesc;
	struct usb_host_interface *cur_altsetting;
	USBperso *device_perso = (USBperso*)filp->private_data; //CHANGER LE NOM POUR PAS QUI CONFONDRE LES 2 AVANT DEVICE MTN device_perso
	struct usb_interface *interface = device_perso->intf;
 	struct usb_device *udev = device_perso->dev;
 	printk(KERN_ALERT"ELE784 -> IOCTL %p\n\r",udev);
	
	switch(cmd){
	
	case IOCTL_GET:
		printk(KERN_ALERT"ELE784 -> IOCTL_GET \n\r");
		break;
	case IOCTL_SET:
		printk(KERN_ALERT"ELE784 -> IOCTL_SET \n\r");
		break;
	case IOCTL_STREAMON: // démarrage de l'aquisition d'une image
		printk(KERN_ALERT"ELE784 -> IOCTL_STREAMON (0x30) \n\r");
		retval = usb_control_msg(udev,
					usb_sndctrlpipe(udev, 0x00),
					0x0B,
					USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE,
					0x0004, 0x0001,NULL,0x00,0x00);
			
		break;
	case IOCTL_STREAMOFF: // arêter l'aquisition d'une image
		printk(KERN_ALERT"ELE784 -> IOCTL_STREAMOFF (0x40) \n\r");
		retval = usb_control_msg(udev,
					usb_sndctrlpipe(udev, 0),
					0x0B,
					USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE,
					0x00, 0x01,NULL,0,0);
			
		break;
	case IOCTL_GRAB: // permet la création des urbs utilent à l'aquisition d'une image
		device->myStatus = 0;
		device->myLengthUsed = 0;
		printk(KERN_ALERT"ELE784 -> IOCTL_GRAB (0x50) \n\r");
		cur_altsetting = interface->cur_altsetting;
		endpointDesc = cur_altsetting->endpoint[0].desc;

		nbPackets = 40;  // The number of isochronous packets this urb should contain			
		myPacketSize = le16_to_cpu(endpointDesc.wMaxPacketSize);			
		size = myPacketSize * nbPackets;
		nbUrbs = 5;

		printk(KERN_ALERT"ELE784 -> IOCTL_GRAB 1 \n\r");
		for (i = 0; i < nbUrbs; ++i) {
		//  usb_free_urb(myUrb[i]); // Pour être certain
		  myUrb[i] = usb_alloc_urb(nbPackets,GFP_ATOMIC);
		  printk(KERN_ALERT"ELE784 -> IOCTL_GRAB 2 \n\r");
		  if (myUrb[i] == NULL) {
		    //printk(KERN_WARNING "");		
		    return -ENOMEM;
		  }

		  //myUrb[i]->transfer_buffer = usb_buffer_alloc(udev,size,GFP_KERNEL,dma);
		  myUrb[i]->transfer_buffer = usb_alloc_coherent(udev,size,GFP_ATOMIC,&(myUrb[i]->transfer_dma));
		  printk(KERN_ALERT"ELE784 -> IOCTL_GRAB 3 \n\r");
		  if (myUrb[i]->transfer_buffer == NULL) {
		    //printk(KERN_WARNING "");		
		    usb_free_urb(myUrb[i]);
		    return -ENOMEM;
		  }
		  printk(KERN_ALERT"ELE784 -> IOCTL_GRAB 4 \n\r");
		  myUrb[i]->dev = udev;
		  myUrb[i]->context = udev;
		  myUrb[i]->pipe = usb_rcvisocpipe(udev, endpointDesc.bEndpointAddress);
		  myUrb[i]->transfer_flags = URB_ISO_ASAP | URB_NO_TRANSFER_DMA_MAP;
		  myUrb[i]->interval = endpointDesc.bInterval;
		  myUrb[i]->complete = complete_callback;
		  myUrb[i]->number_of_packets = nbPackets;
		  myUrb[i]->transfer_buffer_length = size;
		  printk(KERN_ALERT"ELE784 -> IOCTL_GRAB 5 \n\r");
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
//************************************************************************************************************
		       
/* Description: fonction appelé par les urb lorsque complété 
 *
 * Arguments  :  l'urb en question qui appel la fonction 
 *
 * Return     : retourne la valeur selon la commande
 */		       
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
//************************************************************************************************************

module_usb_driver(myUSBdriver);
