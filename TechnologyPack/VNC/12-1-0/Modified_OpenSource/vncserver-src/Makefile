AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/common
vncserver=vncserver
vncserver_LDADD=-L../libvncserver/ -lvncserver

vncserver_LDADD+=-L${SPXLIB}/video -lvideo
vncserver_LDADD+=-L${SPXLIB}/vmedia -lvmedia
vncserver_LDADD+=-L${SPXLIB}/ncml -lncml
vncserver_LDADD+=-L${SPXLIB}/encoding -lencoding
vncserver_LDADD+=-L${SPXLIB}/encryption -lencryption
vncserver_LDADD+=-L${SPXLIB}/blowfish -lblowfish
vncserver_LDADD+=-L${SPXLIB}/token -ltoken
vncserver_LDADD+=-L${SPXLIB}/advisercfg -ladvisercfg
vncserver_LDADD+=-L${SPXLIB}/safesystem -lsafesystem
vncserver_LDADD+=-L${SPXLIB}/featuredefine -lfeaturedefine
vncserver_LDADD+=-L${SPXLIB}/dynamicdrv -ldynamicdrv
vncserver_LDADD+=-L${SPXLIB}/procmanager -lprocmanager
vncserver_LDADD+=-L${SPXLIB}/unix -lunix
vncserver_LDADD+=-L${SPXLIB}/iniparser -liniparser
vncserver_LDADD+=-L${SPXLIB}/dbgout -ldbgout
vncserver_LDADD+=-L${SPXLIB}/libvncserver_extn -lvncserver_extn
vncserver_LDADD+=-L${SPXLIB}/autovideorecord -lautovideorecord
vncserver_LDADD+=-L${SPXLIB}/videorecordcfg -lvideorecordcfg
vncserver_LDADD+=-L${SPXLIB}/datastruct -ldatastruct
vncserver_LDADD+=-L${SPXLIB}/racsessioninfo -lracsessioninfo


vncserver_SOURCES= vncserver.c
noinst_HEADERS= vncserver.h

noinst_PROGRAMS=$(vncserver)
