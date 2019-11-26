/*
 ============================================================================
 Nom         : ioctlcmd.h
 Author      : Samuel Pesant et Mathieu Fournier-Desrochers
 Date 	     : 22-10-2019
 Description : Prototypes de fonctions du pilote série
 ============================================================================
 */

#include <linux/ioctl.h>


#define IOCTL_GET				0x10
#define IOCTL_SET				0x20
#define IOCTL_STREAMON			0x30
#define IOCTL_STREAMOFF			0x40
#define IOCTL_GRAB				0x50
#define IOCTL_PANTILT			0x60
#define IOCTL_PANTILT_RESEST	0x70


