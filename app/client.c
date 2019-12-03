#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stropts.h>
#include <string.h>
#include <linux/ioctl.h>

#include "dht_data.h"
#include "ioctlcmd.h"

int main(int argc, char *argv[]) {
	FILE *foutput;
	int camera_stream, camera_control;
	int bytes, ret;
	unsigned char *inBuffer;
	unsigned char *finalBuf;
	int i;

	inBuffer = malloc((42666)* sizeof(unsigned char));
	finalBuf = malloc((42666 * 2)* sizeof(unsigned char));

	if ((inBuffer == NULL) || (finalBuf == NULL)) {
		return EXIT_FAILURE;
	}

	// Take from arg the camera device
	if (argc > 1) {
	  camera_stream = open(argv[1], O_RDONLY);
	} else {
	  camera_stream = open("/dev/camera_stream1", O_RDONLY);
	}
//	if (argc > 2) {
//	  camera_control = open(argv[2], O_RDONLY);
//	} else {
//	  camera_control = open("/dev/camera_control0", O_RDONLY);
//	}

	// Etape #2
	ret = ioctl(camera_stream, IOCTL_STREAMON);
printf("IOCTL_STREAMON ret = %d\n", ret);
	sleep(2);

//	ret = ioctl(camera_control, IOCTL_PANTILT_RESET);
/*	ret = ioctl(camera_stream, IOCTL_PANTILT_RESET);
printf("IOCTL_PANTILT_RESET ret = %d\n", ret);

	bytes = 3;
	for (i = 0; i < 10; i++) {
//		ret = ioctl(camera_control, IOCTL_PANTILT, &bytes);
		ret = ioctl(camera_stream, IOCTL_PANTILT, &bytes);
	}
printf("IOCTL_PANTILT ret = %d\n", ret);
	bytes = 3;
	for (i = 0; i < 10; i++) {
//		ret = ioctl(camera_control, IOCTL_PANTILT, &bytes);
		ret = ioctl(camera_stream, IOCTL_PANTILT, &bytes);
	}
printf("IOCTL_PANTILT ret = %d\n", ret);
	bytes = 3;
	for (i = 0; i < 10; i++) {
//		ret = ioctl(camera_control, IOCTL_PANTILT, &bytes);
		ret = ioctl(camera_stream, IOCTL_PANTILT, &bytes);
	}
printf("IOCTL_PANTILT ret = %d\n", ret);
	bytes = 3;
	for (i = 0; i < 10; i++) {
//		ret = ioctl(camera_control, IOCTL_PANTILT, &bytes);
		ret = ioctl(camera_stream, IOCTL_PANTILT, &bytes);
	}
printf("IOCTL_PANTILT ret = %d\n", ret);*/

//
//	ret = ioctl(camera, IOCTL_STREAMON);
//printf("IOCTL_STREAMON ret = %d\n", ret);
//	sleep(2);

	foutput = fopen("./fichier.jpg", "wb");
	if (foutput == NULL) {
		return EXIT_FAILURE;
	}
	// Etape #3
	ret = ioctl(camera_stream, IOCTL_GRAB);
printf("IOCTL_GRAB ret = %d\n", ret);
	// Etape #4
	bytes = read(camera_stream, inBuffer, 42666);
	if (bytes <= 0) {
		fprintf(stderr, "unable to read image\n");
		return EXIT_FAILURE;
	}

	// Etape #5
	ret = ioctl(camera_stream, IOCTL_STREAMOFF);
printf("IOCTL_STREAMOFF ret = %d\n", ret);

	memcpy(finalBuf, inBuffer, HEADERFRAME1);
	memcpy(finalBuf + HEADERFRAME1, dht_data, DHT_SIZE);
	memcpy(finalBuf + HEADERFRAME1 + DHT_SIZE, inBuffer + HEADERFRAME1, (bytes - HEADERFRAME1));
	fwrite(finalBuf, bytes + DHT_SIZE, 1, foutput);
	fclose(foutput);
	
	close(camera_stream);
	//close(camera_control);
//
	return EXIT_SUCCESS;
}
