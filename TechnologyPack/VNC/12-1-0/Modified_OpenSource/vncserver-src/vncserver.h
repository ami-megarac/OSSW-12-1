/******************************************************************
******************************************************************
*
* vncserver.h -> contains generic function Implementation for AMI VNC Server
******************************************************************
***                                                            ***
***        (C)Copyright 2018, American Megatrends Inc.         ***
***                                                            ***
***                    All Rights Reserved                     ***
***                                                            ***
***       5555 Oakbrook Parkway, Norcross, GA 30093, USA       ***
***                                                            ***
***                     Phone 770.246.8600                     ***
***                                                            ***
******************************************************************
******************************************************************
******************************************************************
* This is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This software is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this software; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
*  USA.
******************************************************************/
#ifndef VNC_SERVER_H
#define VNC_SERVER_H
#include <rfb/rfbregion.h>
#include <rfb/rfb.h>
#ifdef SOCKET
#undef SOCKET
#endif

#ifdef AMI_VNC_EXTN
#include "vncserver_extn.h"
#include "common.h"
#include "videorecord.h"
#include "ncml.h"
#include "adviser_cfg.h"
#include "soc_hdr_vnc.h"
#endif

/************************Define*******************/

#define FB_FILE 		"/dev/fb0"
//To debug log, change DISABLE to 1
#define DISABLE 0
#ifdef AMI_VNC_EXTN
#define LIB_VIDEO "/usr/local/lib/libvideo_vnc.so"
#define EMPTY_STRING ""

#define INIT_VIDEO_RESOURCE_VNC "init_video_resource_vnc"
#define RUN_XCURSOR_THREAD "run_xcursor_thread"
#define VNC_CAPTURE_CURSOR "vnc_capture_cursor"
#define SET_SEND_CURSOR_PATTERN_FLAG "set_send_cursor_pattern_flag"

#define VNC_BIN_PATH "/usr/local/bin/vncserver"
#define ADVISERD "adviserd"
#endif

#define VNC_PORT 5900
#define LOCALHOST "localhost"
#ifdef AMI_VNC_EXTN
#define DEFER_VIDEO_UPDATE //video update will be sent to client with delay

#endif

/***************************Struct definations*******************/

struct fb_info_t {
	struct fb_fix_screeninfo fix_screen_info;
	struct fb_var_screeninfo var_screen_info;
	int fd;
	void* fbmem;
};

/* This structure is created so that every client has its own pointer */
typedef struct ClientData {
	rfbBool oldButton;
	int oldx, oldy;
} ClientData;

/***********************Global Variable Declarations***********************/
rfbScreenInfoPtr rfbScreen;

#ifdef AMI_VNC_EXTN
static struct fb_info_t fb_info;
static unsigned short int *fbmmap = MAP_FAILED;
SERVICE_CONF_STRUCT	vncConf;

int seqno;
int m_btn_status;
int g_prv_capture_status = 0;
char *emptyScreen = NULL;
#else
int maxx=800,maxy=600;
int actvSessCount = 0;
#endif
static int bpp = 4;

rfbClientPtr pCl = NULL;
#ifdef AMI_VNC_EXTN
pthread_t videoThread = NULL;
pthread_t cursorThread = NULL;
sem_t	mStartCapture;
sem_t	mStartCursorCapture;
#endif
/***********************Function Declarations************************/

/*video functions*/
#ifdef AMI_VNC_EXTN
int (*init_video_resource_vnc)();
int (*run_xcursor_thread_sym)();
int (*vnc_capture_cursor_sym) (void *);
void (*set_send_cursor_pattern_flag_sym)(int);

void vncUnloadResource() ;
void initVideo();
int startCapture();
void* updateVideo();//thread function to maintain video update
rfbBool vncrfbProcessEvents(rfbScreenInfoPtr screen,long usec);
void updateHostCursorPattern(vnc_cursor_t *cursor_info);
void *updateCursor();
void showBlankScreen(char *message,int xPos,int yPos);
void getActiveSocketList(SOCKET* active_sock_list);
void adoptResolutionChange(rfbClientPtr cl);
#endif//AMI_VNC_EXTN
#endif

