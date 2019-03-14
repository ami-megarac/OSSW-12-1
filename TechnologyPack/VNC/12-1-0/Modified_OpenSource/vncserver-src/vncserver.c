/*****************************************************************
******************************************************************
*
* vncserver.c -> contains AMI VNC Server Implementation
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
*  This is free software; you can redistribute it and/or modify
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
#include <rfb/rfb.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <rfb/keysym.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/sysmacros.h> /* For makedev() */
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <libvncserver/private.h>
#include <semaphore.h>
#include <sys/sysinfo.h>

#include "examples/radon.h"
#include "vncserver.h"

#ifdef AMI_VNC_EXTN
#include "fb_vnc_dev/fb_vnc_ioctl.h"
#ifdef TRUE
#undef TRUE
#define TRUE 1
#endif
#include "procreg.h"
#include "unix.h"
#include "hid.h"
#include "session.h"
#include "vmedia_instance.h"
#endif

#ifdef SOCKET
#undef SOCKET
#endif

#ifdef AMI_VNC_EXTN
sem_t	mSessionTimer;
int maxx = VNC_MIN_RESX, maxy = VNC_MIN_RESY; //default video resolution
int pre_maxx = VNC_MIN_RESX, pre_maxy = VNC_MIN_RESY; // holds previous video resolution when showing empty screen
int emptyScreenSize;
pthread_t timerThread;
extern rfbClientIteratorPtr rfbGetClientIteratorWithClosed(rfbScreenInfoPtr rfbScreen);
char titleString[TITLE_STRING_LEN]={0};
char titleString_viewOnly[TITLE_STRING_LEN+VIEW_ONLY_STRING_LEN] ={0};
int isResolutionChanged = 0;
#endif
/*****************************Function Definations*********************/
#ifdef AMI_VNC_EXTN
void
vnc_session_timercall()
{
	double timedout = 0;
	time_t update_time;
	struct sysinfo sys_info;
	rfbClientIteratorPtr i;
	rfbClientPtr cl;

	i = rfbGetClientIteratorWithClosed(rfbScreen);
	cl = rfbClientIteratorHead(i);
	while(cl)
	{
		timedout = 1;
		update_time = 0;
		if(cl->sock != -1){
			update_time = getLastPacketTime(cl->sock);

			if( !sysinfo(&sys_info)) {
				timedout = difftime(sys_info.uptime, update_time);
			}

			//If timeout value is greater the configured sessiont timeout value, send session timeout packet to the particular session
			if(timedout >= vncConf.SessionInactivityTimeout ) {
				if (setTimedOut(cl->sock, 1) != 0)
					TCRIT("\nUnable to update timedout for : %d \n", cl->sock);
			}
			cl=rfbClientIteratorNext(i);
		}
		if(actvSessCount <= 0)
			break;
	}
	rfbReleaseClientIterator(i);
	return;
}

void rfbDrawStringPerLine(rfbScreenInfoPtr rfbScreen,rfbFontDataPtr font,
		int x,int y,const char* string,rfbPixel colour)
{

	while(*string) {
		// on new line character, modify x and y to move the text to next line.
		if(*string == NEW_LINE) {
			// draw starting from left side of the screen.
			// x is 2 so that there is some space between left margin of the screen and text.
			x  = SCREEN_POS_X;
			// move to next line.
			y += SCREEN_POS_Y;
		}
		x+=rfbDrawChar(rfbScreen,font,x,y,*string,colour);
		string++;
	}
}

void setupFb(struct fb_info_t* fb_info) {

	int fd = -1;

	fd = open(FB_FILE, O_RDONLY);
	if (fd < 0) {
		printf("Open %s error!\n", FB_FILE);
		onStartStopVnc(STOP);
		return;
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_info->fix_screen_info) < 0) {
		printf("\n FBIOGET_FSCREENINFO failes \n");
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_info->var_screen_info) < 0) {
		printf("\n FBIOGET_VSCREENINFO failes \n");
	}

	fb_info->fbmem = mmap(0, fb_info->fix_screen_info.smem_len, PROT_READ, MAP_PRIVATE, fd, 0);
	fbmmap = fb_info->fbmem;
	close(fd);
	if (fbmmap == MAP_FAILED) {
		printf("\nmmap failed\n");
		vncUnloadResource();
		onStartStopVnc(STOP);
		exit(EXIT_FAILURE);
	}
	emptyScreen= (char*)malloc(maxx*maxy*bpp);
	emptyScreenSize = maxx*maxy*bpp;
}

void fbUnmap(void) {
	munmap(fb_info.fbmem, fb_info.fix_screen_info.smem_len);
}
#endif//AMI_VNC_EXTN

static void clientgone(rfbClientPtr cl) {
#ifdef AMI_VNC_EXTN
	SOCKET* active_sock_list = NULL;
	if ( cl->clientData != NULL ) {
		//Check for master session exit
		if(cl->viewOnly == FALSE){
			g_master_present = MASTER_GONE; //confirm master exit so mark as gone.
		}
		actvSessCount--;
		setActiveSessionCount(actvSessCount);//updating activesession count in extension pakages
	}

	if(active_sock_list == NULL){
		active_sock_list = (SOCKET *)malloc((sizeof(SOCKET) * actvSessCount));
	}
	//get the list of active sessions
	getActiveSocketList(active_sock_list);
	
	//When a client is closed, use the list of active sockets to identify which session needs to be cleared.
	onClientGone(active_sock_list); // Client gone handler for extension package.
	if(active_sock_list != NULL){
		free(active_sock_list);
		active_sock_list = NULL;
	}

	if(actvSessCount == 0)
	{
		memset((char *)sessionInfo, 0, (sizeof(VNCSessionInfo) * vnc_max_session));
		//Reset to default setting when all of the VNC session gone.
		maxx=VNC_MIN_RESX;
		maxy= VNC_MIN_RESY;
		if(rfbScreen->frameBuffer != emptyScreen){
			rfbScreen->frameBuffer = emptyScreen;
		}
		adoptResolutionChange( cl);
		//update resolution in library file. This will avoid vnc 
		setResolution(maxx,maxy);
		on_vnc_no_session();
	}
#endif//AMI_VNC_EXTN
	free(cl->clientData);
	cl->clientData = NULL;
}

#ifdef AMI_VNC_EXTN
void adoptResolutionChange(rfbClientPtr cl) {

	rfbPixelFormat prevFormat;
	rfbBool updateFormat = FALSE;
	rfbClientIteratorPtr iterator;
	
	//update screen information and intimate all active clients
	prevFormat = cl->screen->serverFormat;

	//resolution changed, so get new resouliton.
	if( isResolutionChanged ) {
		getResolution(&maxx,&maxy);
		isResolutionChanged = 0;
	}

	if (maxx & 3)
		printf("\n updated width [%d] is not a multiple of 4.\n", maxx);

	cl->screen->width= maxx;
	cl->screen->height = maxy;
	cl->screen->bitsPerPixel = cl->screen->depth = 8*bpp;
	cl->screen->paddedWidthInBytes = maxx*bpp;

	//rfbInitServerFormat(screen, bitsPerSample);

	if (memcmp(&cl->screen->serverFormat, &prevFormat,
	     sizeof(rfbPixelFormat)) != 0) {
		updateFormat = TRUE;
	}

	/* update pointer values */

	if (cl->screen->cursorX >= maxx)
		cl->screen->cursorX = maxx - 1;
	if (cl->screen->cursorY >= maxy)
		cl->screen->cursorY = maxy - 1;

	/* intimate active clients */
	iterator = rfbGetClientIterator(cl->screen);
	while ((cl = rfbClientIteratorNext(iterator)) != NULL) {

		if (updateFormat)
			  cl->screen->setTranslateFunction(cl);

		LOCK(cl->updateMutex);
		sraRgnDestroy(cl->modifiedRegion);
		cl->modifiedRegion = sraRgnCreateRect(0, 0, maxx, maxy);
		sraRgnMakeEmpty(cl->copyRegion);
		cl->copyDX = 0;
		cl->copyDY = 0;

		if (cl->useNewFBSize)
			cl->newFBSizePending = TRUE;

		TSIGNAL(cl->updateCond);
		UNLOCK(cl->updateMutex);
	}
	rfbReleaseClientIterator(iterator);
	//update maxRectColumn for framerectfromtiles
	maxRectColumn = (abs(maxx/TILE_WIDTH)) /2;
}
#endif//AMI_VNC_EXTN

static enum rfbNewClientAction newclient(rfbClientPtr cl) {
#ifdef AMI_VNC_EXTN
	struct sysinfo sys_info;
	if ( g_kvm_client_state == VNC ) {
		//Check for maximum session reach
		if(actvSessCount >= vnc_max_session){
			rfbCloseClient(cl);
			return RFB_CLIENT_REFUSE;
		}
#endif
		cl->clientData = (void*) calloc(sizeof(ClientData), 1);
		cl->clientGoneHook = clientgone;
#ifdef AMI_VNC_EXTN
		cl->enableSupportedMessages = TRUE;
		seqno = 0;
		m_btn_status = 0;
		pCl = cl;

		if (pVidHandle) {
			if(!create_vnc_fake_session) {
				create_vnc_fake_session = dlsym(pVidHandle, CREATE_VNC_FAKE_SESSION);
				if (!create_vnc_fake_session)
					PRINT_DLFCN_ERR;
			}
			if (create_vnc_fake_session)
				create_vnc_fake_session();
		}

		if(actvSessCount <0)
			actvSessCount = 1;
		else
			actvSessCount++;
		/*new incoming session so post to resume update video thread.
		if there is only one session then wakeup thread else it will
		be already running.
		*/
		setActiveSessionCount(actvSessCount);//updating activesession count in extension pakages
		if(actvSessCount == 1){
			updateUsbStatus(1);//enable usb devices
			sem_post(&mStartCapture);
			sem_post(&mSessionTimer);
			g_master_present = MASTER_PRESENT;
			if(updateTitleString(titleString, FALSE) == 0)
				cl->screen->desktopName = titleString;
		}
		else {
		
			/*If second session comes ,
			*always check whether master session avaible before giving permission.
			*may be in previsous session master might exited when active "view-only" was present.
			*current active session might be a "view-only".
			*if So incoming session will become a master.
			*/
			if(g_master_present == MASTER_PRESENT){
				//Master session avaible.
				cl->viewOnly = TRUE;//By setting TRUE we are making it a "view-only"
				if(updateTitleString(titleString_viewOnly, TRUE) == 0)
					cl->screen->desktopName = titleString_viewOnly;
			}
			else	{	//master is not present
				cl->viewOnly = FALSE; //by setting FALSE ,we are making him a master
				g_master_present = MASTER_PRESENT;//Set master present.
				if(updateTitleString(titleString, FALSE) == 0)
					cl->screen->desktopName = titleString;
			}
		}

		if(actvSessCount > 0) {
			// Suspend continuous recording if enabled
			if(g_corefeatures.record_pre_boot_or_crash_video == ENABLED)
				g_sync_rec_jviewer = 1;
			/* If incoming connection is granted with master privilege, then we
			** have to wake up thread to capture cursor pattern change.
			** (Applicable only if hardware cursor feature is enabled) */
			if((cl->viewOnly == FALSE) && (vnc_capture_cursor_sym))
				sem_post(&mStartCursorCapture);
		}
		if(!sysinfo(&sys_info))
		{
			if(setSessionInfo(cl->sock, sys_info.uptime, cl->host) != 0)
				TCRIT("\nerror while setting session info...\n");
		}
		return RFB_CLIENT_ACCEPT;
	}
	else
	{
		//TODO:close connection as VNC is not selected as KVM client
		rfbCloseClient(cl);
		return RFB_CLIENT_REFUSE;
	}
#endif//AMI_VNC_EXTN

	return RFB_CLIENT_ACCEPT;
}

#ifdef AMI_VNC_EXTN
int powerCons = 1;
/* Here the pointer events are handled */
static void handleMouseEvent(int buttonMask, int x, int y, rfbClientPtr cl) {
	struct sysinfo sys_info;
	if((powerCons == 0) && (cl->viewOnly == FALSE)){ //Process only for master session
		mousePkt(buttonMask, x, y,maxx,maxy);
		if(!sysinfo(&sys_info))
		{
			if(setLastPacketTime(cl->sock, sys_info.uptime) != 0)
				TCRIT("\nUnable to update last packet time\n");
		}
	}
}

void handlekeyEvent(rfbBool down, rfbKeySym key, rfbClientPtr cl) {
	struct sysinfo sys_info;
	if((powerCons == 0) && (cl->viewOnly == FALSE)){ //Process only for master session
		handleKbdEvent(down, key);
		if(!sysinfo(&sys_info))
		{
			if(setLastPacketTime(cl->sock, sys_info.uptime) != 0)
				TCRIT("\nUnable to update last packet time\n");
		}
	}
}

static void init_fb_server(int argc, char **argv) {
	if (rfbScreen == NULL) {
		rfbScreen = rfbGetScreen(&argc, argv, maxx, maxy, 8, 3, bpp);
		if (!rfbScreen)
			return;
		rfbPixelFormat pixfmt = { 32, //U8  bitsPerPixel;
				32, //U8  depth;
				0, //U8  bigEndianFlag;
				1, //U8  trueColourFlag;
				255, //U16 redMax;
				255, //U16 greenMax;
				255, //U16 blueMax;
				16, //U8  redShift;
				8, //U8  greenShift;
				0, //U8  blueShift;
				0, //U8  pad 1;
				0 //U8  pad 2
				};
		rfbScreen->serverFormat = pixfmt;
		rfbScreen->alwaysShared = TRUE;
		rfbScreen->frameBuffer = (char*) fb_info.fbmem;//(char*)vncbuf;
		rfbScreen->ptrAddEvent = handleMouseEvent;
		rfbScreen->kbdAddEvent = handlekeyEvent;/* Here the key events are handled */
		rfbScreen->newClientHook = newclient;
		rfbScreen->httpDir = NULL;
		rfbScreen->httpEnableProxyConnect = TRUE;
		//Make vnc to listen to user specified port value.
		if( g_nonsecureport != 0 ){
			rfbScreen->ipv6port = rfbScreen->port = g_nonsecureport;
		}
		/* Check if bit value for secure connection is set.
		** If so listen in loopback interface alone. */
		switch (vncGetListenInterface())
		{
			case INADDR_LOOPBACK:
				rfbScreen->listenInterface = htonl(INADDR_LOOPBACK);
				rfbScreen->listen6Interface = LOCALHOST;
				break;
			case INADDR_NONE:
				TCRIT("Error: Unable to get listenInterface. Stopping VNC server.\n");
				vncUnloadResource();
				onStartStopVnc(STOP);
				break;
			case INADDR_ANY: /* Default listen interface */
			default:
				break; /* nothing to do !! */
		}
		rfbInitServer(rfbScreen);
	}
}

void vncUnloadResource() {
	fbUnmap();
	if(emptyScreen != NULL)
		free(emptyScreen);
	rfbScreenCleanup(rfbScreen);
	if(sessionInfo != NULL)
		free(sessionInfo);

	if(vnc_session_guard != NULL)
		free(vnc_session_guard);

	//If adviserd service is running, then it may use usb resource. So don't release usb resource.
	if( IsProcRunning(ADVISERD) == 0 ) {
		CloseUSBDevice();
	}

	if (vncpipe != -1) {
		closeVncPipe(vncpipe);
	}

}
/* Initialization */
int main(int argc, char** argv) {
	memset(&vncConf, 0, sizeof(SERVICE_CONF_STRUCT));
	/* Retrieves the Enabled features */
	RetrieveCoreFeatures(&g_corefeatures);
	//get VNC configuration
	if ( get_service_configurations(VNC_SERVICE_NAME, &vncConf) < 0) {
		TCRIT("\n unable to get VNC service configuration ");
		exit(0);
	}
	//Check VNC service state
	if( !vncConf.CurrentState ){
		TCRIT("\n VNC is not enabled ");
		exit(0);
	}
	//check if port value is set other than default value.
	if ( ( vncConf.NonSecureAccessPort != VNC_PORT ) && ( vncConf.NonSecureAccessPort != -1 ) ) {
		g_nonsecureport = vncConf.NonSecureAccessPort;
	}
	if( g_corefeatures.adviser_support == ENABLED ) {
		g_kvm_client_state = GetKVMClientOption();
		if( ( g_kvm_client_state != ADVISER ) && ( g_kvm_client_state != VNC ) ) {
			TCRIT("Error: Unable to get KVM client option %d",g_kvm_client_state);
			exit(0);
		}
	}
	/*Checking for VNC MaxAllowSession*/
	if (isNotApplicable((unsigned char *)&(vncConf.MaxAllowSession), 
		sizeof(vncConf.MaxAllowSession))) {
		TCRIT("VNC Session, MaxAllowSession data is not applicable\n");
		exit(0);
	} else {
		getNotEditableData((unsigned char *)&(vncConf.MaxAllowSession), 
			sizeof(vncConf.MaxAllowSession), 
			(unsigned char *)&(vnc_max_session));
		setVncmaxSession(vnc_max_session);//updating Vnc maxsessions  in extension pakages
		TDBG("VNC Session, MaxAllowSession value::%d\n", vnc_max_session);
	}

	onStartStopVnc(START);
	//disable default rfb log
	rfbLogEnable(DISABLE);
	setupFb(&fb_info);
	initVideo();
	if (init_usb_resources() < 0) {
		printf("\nfailed init_usb_resources\n"); fflush(stdout);
		vncUnloadResource();
		onStartStopVnc(STOP);
	}
	if (save_usb_resource() < 0) {
		printf("\nfailed save_usb_resource\n"); fflush(stdout);
		vncUnloadResource();
		onStartStopVnc(STOP);
	}

	/* initialize the server */
	init_fb_server(argc, argv);

	vnc_session_guard = getVncSessionGuard();
	// vnc_session_guard = (pthread_mutex_t *)calloc(vnc_max_session, sizeof(pthread_mutex_t));
	if (vnc_session_guard == NULL)
	{
		TCRIT("unable to allocate resource for vnc session guard\n");
		vncUnloadResource();
		onStartStopVnc(STOP);
	}
	

	if (rfbScreen == NULL) {
		printf("\nUnable to init_fb_server. quitting the application.\n");
		vncUnloadResource();
		onStartStopVnc(STOP);
	}

	// sessionInfo = (VNCSessionInfo *) malloc ((sizeof(VNCSessionInfo) * vnc_max_session));
	sessionInfo = getSessionInfo();
	if (sessionInfo == NULL)
	{
		TCRIT("unable to allocate resource for vnc session info\n");
		vncUnloadResource();
		onStartStopVnc(STOP);
	}

	if (sem_init (&mStartCapture, 0, 0) < 0){ 
		vncUnloadResource();
		onStartStopVnc(STOP);
	}

	if (sem_init (&mStartCursorCapture, 0, 0) < 0) { 
		vncUnloadResource();
		onStartStopVnc(STOP);
	}

	if (sem_init (&mSessionTimer, 0, 0) < 0) {
		vncUnloadResource();
		onStartStopVnc(STOP);
	}
	/* Implement our own loop to detect changes in the video and transmit to client. */
	if (0 != pthread_create (&videoThread, NULL,updateVideo,NULL))
	{
		printf("\n Thread creation failed...\n");
		vncUnloadResource();
		onStartStopVnc(STOP);
	}

	if (0 != pthread_create(&timerThread, NULL,checkTimer,NULL))
	{
		vncUnloadResource();
		startStopVnc(STOP);
	}


	if (initVideoRecordingResources() < 0) {
		vncUnloadResource();
		onStartStopVnc(STOP);
	}

	vncpipe = openVncPipe(VNC_CMD_PIPE);

	if (vncpipe == -1) {
		vncUnloadResource();
		onStartStopVnc(STOP);
	}

	if (0 != pthread_create (&cmdThread,NULL,checkIncomingCmd,NULL))
	{
		TCRIT("Error: VNC cmdThread creation failed.\n");
		vncUnloadResource();
		onStartStopVnc(STOP);
	}

	if (0 != pthread_create(&cursorThread, NULL,updateCursor,NULL))
	{
		vncUnloadResource();
		onStartStopVnc(STOP);
	}

	while (1) {
		while (rfbScreen->clientHead == NULL)
			rfbProcessEvents(rfbScreen, 10000);		
		
		vncrfbProcessEvents(rfbScreen, 1000);
	}
	vncUnloadResource();
	onStartStopVnc(STOP);
	return (0);
}
#else

int main(int argc,char** argv)
{
  rfbScreenInfoPtr rfbScreen = rfbGetScreen(&argc,argv,maxx,maxy,8,3,bpp);
  if(!rfbScreen)
    return 0;
  rfbScreen->desktopName = "LibVNCServer Example";
  rfbScreen->frameBuffer = (char*)malloc(maxx*maxy*bpp);
  rfbScreen->alwaysShared = TRUE;
//  rfbScreen->ptrAddEvent = doptr;
//  rfbScreen->kbdAddEvent = dokey;
  rfbScreen->newClientHook = newclient;
  rfbScreen->httpDir = "../webclients";
  rfbScreen->httpEnableProxyConnect = TRUE;

//  initBuffer((unsigned char*)rfbScreen->frameBuffer);
  rfbDrawString(rfbScreen,&radonFont,20,100,"Hello, World!",0xffffff);

  /* This call creates a mask and then a cursor: */
  /* rfbScreen->defaultCursor =
       rfbMakeXCursor(exampleCursorWidth,exampleCursorHeight,exampleCursor,0);
  */

//  MakeRichCursor(rfbScreen);

  /* initialize the server */
  rfbInitServer(rfbScreen);

#ifndef BACKGROUND_LOOP_TEST
#ifdef USE_OWN_LOOP
  {
    int i;
    for(i=0;rfbIsActive(rfbScreen);i++) {
      fprintf(stderr,"%d\r",i);
      rfbProcessEvents(rfbScreen,100000);
    }
  }
#else
  /* this is the blocking event loop, i.e. it never returns */
  /* 40000 are the microseconds to wait on select(), i.e. 0.04 seconds */
  rfbRunEventLoop(rfbScreen,40000,FALSE);
#endif /* OWN LOOP */
#else
#if !defined(LIBVNCSERVER_HAVE_LIBPTHREAD)
#error "I need pthreads for that."
#endif

  /* this is the non-blocking event loop; a background thread is started */
  rfbRunEventLoop(rfbScreen,-1,TRUE);
  fprintf(stderr, "Running background loop...\n");
  /* now we could do some cool things like rendering in idle time */
  while(1) sleep(5); /* render(); */
#endif /* BACKGROUND_LOOP */

  free(rfbScreen->frameBuffer);
  rfbScreenCleanup(rfbScreen);

  return(0);
}
#endif//AMI_VNC_EXTN

#ifdef AMI_VNC_EXTN
/******************Video update code**********/
void initVideo() {
	int status = 0;
	if (!pVidHandle) {
		pVidHandle = dlopen((char *) LIB_VIDEO, RTLD_NOW);
		if(!pVidHandle)	{
			PRINT_DLFCN_ERR;
			status = -1;
		}
	}

	// If symbol init_video_resource_vnc is availble then already video
	// resources are initialized. No need to call in this case.
	if ((status != -1) && (!init_video_resource_vnc)) {
		init_video_resource_vnc = dlsym(pVidHandle, INIT_VIDEO_RESOURCE_VNC);
		if (!init_video_resource_vnc) {
			PRINT_DLFCN_ERR;
			status = -1;
		} else {

			if(init_video_resource_vnc() < 0) {
				TCRIT("Unable to Open Video Capture Driver. Terminating VNC server\n");
				status = -1;
			}
		}
	}
	/* Captures hardware cursor data */
	if ((status != -1) && (!run_xcursor_thread_sym)) {
		run_xcursor_thread_sym = dlsym(pVidHandle, RUN_XCURSOR_THREAD);
		if (!run_xcursor_thread) {
			PRINT_DLFCN_ERR;
		} else {
			if(run_xcursor_thread_sym() < 0) {
				TCRIT("Unable to create hardware cursor thread\n");
			}
		}
	}
	/* Fetches captured hardware cursor data */
	if ((status != -1) && (!vnc_capture_cursor_sym)) {
		vnc_capture_cursor_sym = dlsym(pVidHandle, VNC_CAPTURE_CURSOR);
		if (!vnc_capture_cursor_sym)
			PRINT_DLFCN_ERR;
	}

	/* set the flag to get full cursor patter [ pilot specific ] */
	if ((status != -1) && (!set_send_cursor_pattern_flag_sym)) {
		set_send_cursor_pattern_flag_sym = dlsym(pVidHandle, SET_SEND_CURSOR_PATTERN_FLAG);
		if (!set_send_cursor_pattern_flag_sym)
			PRINT_DLFCN_ERR;
	}

	if (status == -1) {
		vncUnloadResource();
		onStartStopVnc(STOP);
	}
}

void onStartStopVnc(int mode) {
	int retval = startStopVnc(mode);
	if (mode == START && retval != -1) {
		if (ProcMonitorRegister(VNC_BIN_PATH, (g_nonsecureport > 0 ? g_nonsecureport : VNC_PORT),
				VNC_SERVER, vncSignalHandler, 0) != 0) {
			TCRIT("Unable to register with process monitor. Stopping VNC server.\n");
			retval = -1;
		}
	}
	if (retval == -1 || mode == STOP)
		exit(1);
}

void vncSignalHandler(int signum) {
	switch (signum) {
	// This signal is for, when we are stopping this vnc server under non-configuration changes
	case SIGHUP:
	case SIGKILL:
		// This signal is for, when any configuration change occurs in BMC
	case SIGUSR1:
		/* In case of active session, stop takes more time so service will get stopped before
		** clearing entry in ProcMonitor. It will result in continuous re-spawn of VNC process.
		** To avoid this following fix is added. */
		if(actvSessCount > 0) { // Not needed when no active session.
			deRegisterVnc();
		}
		vncUnloadResource();
		onStartStopVnc(STOP);
		break;
		// This signal is for, when we want to start auto-video recording
	case SIGUSR2:
		// This signal is for, when we receive any service information change and to send it to clients
	case SIGALRM:
	default:
		break;
	}
}

int startCapture()
{	

	int cap_status = VIDEO_ERROR ;
	struct rectUpdate_t recUpdate[maxRectColumn];
#ifndef FRAME_RECT_FORM_TILES
	int height=0,width=0,i=0,pos= sizeof( unsigned short );
	int posx=0,posy=0;
	int x=0,y=0;
	int tilecount = 0;
#else
	int j = 0;
	int count = 0;
#endif //FRAME_RECT_FORM_TILES
	cap_status = videoCapture();
	if(cap_status == VIDEO_SUCCESS)
	{
	// if cap_status is > 1000 dont calculate frame rect from tiles.
	// TODO!!!!
#ifdef FRAME_RECT_FORM_TILES
	count = frameRectFromTiles(&recUpdate[0]);
	for (j=0;j<count;j++){
		rfbMarkRectAsModified(rfbScreen,recUpdate[j].x,recUpdate[j].y,recUpdate[j].width,recUpdate[j].height);
	}

#else
	tilecount = *((unsigned short*)tile_info);

	for (i=0;i<tilecount;i++)
	{
		x = (*(tile_info+pos+1));
		y = (*(tile_info+pos));
		posx = x *TILE_WIDTH;
		posy = y *TILE_HEIGHT;
		width = posx+TILE_WIDTH;
		height =posy+TILE_HEIGHT;
		rfbMarkRectAsModified(rfbScreen,posx,posy,width,height);
		pos +=2;
	}
#endif//FRAME_RECT_FORM_TILES
	}
	return cap_status;
}
#endif//AMI_VNC_EXTN

rfbBool
vncrfbProcessEvents(rfbScreenInfoPtr screen,long usec)
{
	if(usec<0)
		usec = screen->deferUpdateTime*1000;

	rfbCheckFds(screen,usec);
	rfbHttpCheckFds(screen); 

	return TRUE;
}

#ifdef AMI_VNC_EXTN
void* updateVideo()
{
	pthread_t self;
	int capture = VIDEO_ERROR, cleanUp = 0;
	rfbClientIteratorPtr i;
	rfbClientPtr cl;
	struct sysinfo incoming,current;
	double timedout = 0;	

	maxRectColumn = (abs(maxx/TILE_WIDTH)) /2;
	while(1){
		sem_wait(&mStartCapture);
		sysinfo(&incoming);
		powerCons  = (g_corefeatures.power_consumption_virtual_device_usb ? IsPowerconsumptionmode() : 0);
		while(1){

			if( actvSessCount <= 0)
			{
				TINFO("VNC client(s) exited...\n");
				g_prv_capture_status = 0;
				capture=VIDEO_ERROR;
				break;
			}
			//Need to display HID inititlation info in vnc client
			if(powerCons == 1){
				if( !sysinfo(&current)) {
					timedout = difftime(current.uptime, incoming.uptime);
				}
				
				if(timedout <= 5){
					if(g_prv_capture_status != capture){
						//X and Y position has been calculated accordingly to show text in middle of the screen
						//showBlankScreen(HID_INITILIZATION,(VNC_MIN_RESX/2) - (sizeof(HID_INITILIZATION)),VNC_MIN_RESY/2);
						showBlankScreen(HID_INITILIZATION,(maxx/2) - 100,maxy/2);
					}
				}
				else{
					powerCons = 0;
					if(rfbScreen->frameBuffer != (char*) fb_info.fbmem){
						rfbScreen->frameBuffer = (char*) fb_info.fbmem;
					}
				}
			}
			else{
				capture = startCapture() ;

				if(RESOLUTION_CHANGED == capture){
					isResolutionChanged = 1;
					adoptResolutionChange(pCl);
				}
				else if(VIDEO_ERROR == capture || VIDEO_FULL_SCREEN_CHANGE == capture){
					rfbMarkRectAsModified(rfbScreen, 0, 0, maxx, maxy);
				}
				else if(VIDEO_SUCCESS == capture){
					if(isTextMode)
					{
						showBlankScreen((char*)tile_info, SCREEN_POS_X, SCREEN_POS_Y);
					}
					else if(rfbScreen->frameBuffer != (char*) fb_info.fbmem){
						rfbScreen->frameBuffer = (char*) fb_info.fbmem;
						// restore previous resolution and call resolution change when switching between textmode/hid/blankscreen and video
						maxx = pre_maxx;
						maxy = pre_maxy;
						isResolutionChanged = 1;
						adoptResolutionChange(pCl);
					}
				}
				else if(VIDEO_NO_SIGNAL== capture){
					if(g_prv_capture_status != capture){
						showBlankScreen(EMPTY_STRING,VNC_MIN_RESX/2,VNC_MIN_RESY/2);
					}
				}
				
				if( g_prv_capture_status == VIDEO_NO_SIGNAL && ( capture != VIDEO_NO_SIGNAL && capture != RESOLUTION_CHANGED ) ) {
		            //On no signal, video driver will reset the cursor pattern. So set send_xcursor_packet flag to fetch cursor pattern from driver on video update
					if( set_send_cursor_pattern_flag_sym ) {
						set_send_cursor_pattern_flag_sym(TRUE);
					}
				}
			}
			g_prv_capture_status =capture;

			i = rfbGetClientIteratorWithClosed(rfbScreen);
			cl=rfbClientIteratorHead(i);
			while(cl) {
#ifdef DEFER_VIDEO_UPDATE
				rfbUpdateClient(cl);
#else
				if (cl->sock >= 0 && !cl->onHold && FB_UPDATE_PENDING(cl) &&
				    !sraRgnEmpty(cl->requestedRegion)) {
					rfbSendFramebufferUpdate(cl,cl->modifiedRegion);
				}
#endif
				//This function call will help to identify the rfb client which has to be closed in the case of
				//session timeout/session disconnect. This function call will check if there is a session with socket fd 
				//which matches the client socket fd, and whether the status is set to TIMED_OUT, or DISCONNECTED of that
				//session. If so isCleanUpSession() will return true.
				cleanUp = isCleanUpSession(cl->sock);
				//Close the active VNC clients, if g_kvm_client_state is other than VNC
				if ( ( ( g_corefeatures.adviser_support) &&( g_kvm_client_state != VNC ) && ( cl->sock != -1 ) )
						|| ( (cl->sock == -1) && (cl->clientData != NULL)) || (true == cleanUp)) {
					rfbCloseClient(cl);
					rfbClientConnectionGone(cl);
					cleanUp = false;
				}
				cl=rfbClientIteratorNext(i);
			}
			//set Host keyboard LED status in vnc server. 
			SetLEDStatus();
			rfbReleaseClientIterator(i);
			select_sleep(0,10000);// make cpu happy , increasing this value will impact video smooth
		}
	}

	self = pthread_self();
	pthread_detach(self);
	pthread_exit(NULL);
	return NULL;
}


/* Updates host cursor pattern for VNC client.
**
** Note: Video driver gives pattern change and color
** information. So simply updating the values in libvncserver
** cursor structure.
**
** (parameter information)
** cursor_info - Cursor data from video dirver
*/
void updateHostCursorPattern(vnc_cursor_t *cursor_info)
{
	rfbCursorPtr c;
	/* update cursor pattern data only in case of proper cursor width & height values */
	if ((cursor_info != NULL) && (cursor_info->width <= 64) && (cursor_info->height <= 64))
	{
		/* Using same pattern for calculating mask and source. Leaving mask as
		** NULL will create additional border around existing cursor pattern.*/
		c = rfbMakeXCursor(cursor_info->width,cursor_info->height,(char *)&cursor_info->pattern[0],(char *)&cursor_info->pattern[0]);
		c->richSource = (unsigned char *)&cursor_info->color[0]; // Gives cursor color!!!!
		/* Aligns given cursor image to top-left position. */
		c->xhot = cursor_info->x_offset;
		c->yhot = cursor_info->y_offset;
		rfbSetCursor(rfbScreen, c);
	}
}
/* Thread function to fetch cursor data from libvideo_vnc */
void *updateCursor()
{
	pthread_t self;
	/* Following buffers will be initialized inside libvideo_vnc.
	** Since change in cursor pattern requies reinitialization,
	** it is performed inside libvideo_vnc. */
	vnc_cursor_t cursor;

	if (vnc_capture_cursor_sym)
	{
		while (1) {
			/* Cursor pattern change information will be updated only if master
			** session is active.
			** TODO: Clear cursor image for video only session with active master session.
			** Following code clears cursor image if
			** 1. Video only session alone is active.
			** 2. Recieved video capture error / no signal. (To prevent cursor image scramble)*/
			if((g_master_present == MASTER_GONE) ||
			(g_prv_capture_status == VIDEO_ERROR || g_prv_capture_status == VIDEO_NO_SIGNAL))
			{
				memset(&cursor.pattern, ' ', sizeof(cursor.pattern));
				memset(&cursor.color, 0, sizeof(cursor.color));
				cursor.height = cursor.width = cursor.x_offset = cursor.y_offset = 0; 
				updateHostCursorPattern(&cursor);

				if (g_master_present == MASTER_GONE)
					sem_wait(&mStartCursorCapture);
			}
			else {
				// Update only if cursor pattern is changed
				if(vnc_capture_cursor_sym((void *)&cursor) > 0)
					updateHostCursorPattern(&cursor);
			}
			select_sleep(0,10000); 
		}
	}
	else
		TCRIT("Unable to initialize cursor update thread !!!");

	self = pthread_self();
	pthread_detach(self);
	pthread_exit(NULL);
	return NULL;
}

/* Function to switch vnc screen to blank screen. If message is passed will be used to display it */
void showBlankScreen(char *message,int xPos,int yPos) {
	char formattedString[TEXT_LEN] = {0};
	int resChanged = 0;
	if(rfbScreen->frameBuffer != emptyScreen){
		rfbScreen->frameBuffer = emptyScreen;

		if(maxx != VNC_MIN_RESX)
		{
			pre_maxx = maxx;
			maxx = VNC_MIN_RESX;
			resChanged = 1;
		}
		if(maxy != VNC_MIN_RESY)
		{
			pre_maxy = maxy;
			maxy = VNC_MIN_RESY;
			resChanged = 1;
		}
		if(resChanged)
			adoptResolutionChange(pCl);
	}

	memset(emptyScreen, 0, emptyScreenSize);

	if(message != NULL && (message[0] != '\0')) {

		if(isTextMode)
		{
			// clear destination string and screen.
			memset(formattedString, 0, TEXT_LEN);
			// Format the text to include newline
			formatString(message, formattedString);
			// paint the formatted string
			rfbDrawStringPerLine(rfbScreen,&radonFont,xPos,yPos,formattedString,0xffffff);
		}
		else
		{
			rfbDrawString(rfbScreen,&radonFont,xPos,yPos,message,0xffffff);
		}
		rfbMarkRectAsModified(rfbScreen,0,0,maxx,maxy);
	}
	return ;
}

/**
 * Get the list of active sockets.
 * 
 * @param active_sock_list - the array of active sockets.
 * 
 */
void getActiveSocketList(SOCKET* active_sock_list){
	
	rfbClientIteratorPtr itr;
	rfbClientPtr cli;
	int count = 0;
	if(active_sock_list == NULL)
		return;
	itr = rfbGetClientIteratorWithClosed(rfbScreen);
	cli = rfbClientIteratorHead(itr);
	while(cli)
	{
		if(count > actvSessCount)
			break;
		active_sock_list[count] = cli->sock;
		count++;
		cli=rfbClientIteratorNext(itr);
	}
	rfbReleaseClientIterator(itr);
}
#endif

/**
 * checkTimer - calls vnc_session_timercall on regular interval (VNC_TIMER_TICKLE) seconds
 * to check if the given sessions are timedout or not.
 * 
 */
#ifdef AMI_VNC_EXTN
void *checkTimer()
{
	pthread_t self;
	int SleepIntervalSecs = VNC_TIMER_TICKLE;
	int timeleftSecs;
	prctl(PR_SET_NAME,__FUNCTION__,0,0,0);

	while(1)
	{
		// wait till a session is registered
		sem_wait(&mSessionTimer);

		while(1)
		{
			timeleftSecs = SleepIntervalSecs;
			while(timeleftSecs)
			{
				timeleftSecs = sleep(timeleftSecs);
			}

			//call the Timer
			vnc_session_timercall();

			if(actvSessCount <= 0)
				break;
		}
	}

	self = pthread_self();
	pthread_detach(self);
	pthread_exit(NULL);
	return NULL;
}
#endif
