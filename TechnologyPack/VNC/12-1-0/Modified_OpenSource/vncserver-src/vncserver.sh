#!/bin/sh
# /etc/init.d/vncserver.sh: start VNC server application
#
# chkconfig: 2345 10 90
#Runlevel : 3 = S19
#Runlevel : 6 = K19
#Runlevel : 7 = K19
#Runlevel : 8 = K19
# Restart the service on warm reboot
#Runlevel : 9 = K30
#Runlevel : 9 = S30

PATH=/bin:/usr/bin:/sbin:/usr/sbin

VNC_SERVER=/usr/local/bin/vncserver
ACTION=$1


test -f /usr/local/bin/vncserver || exit 0

# Options for start/restart the daemon

case "$ACTION" in
  start)
    echo "Starting VNC Server"
    start-stop-daemon --start --quiet --exec $VNC_SERVER
    ;;
  stop)
    echo -n "Stopping VNC Server"
    start-stop-daemon --stop --quiet --exec $VNC_SERVER --signal HUP
    echo "."
    ;;
    reload)
	echo -n "Reloading VNC Server"
	start-stop-daemon --stop --quiet --exec $VNC_SERVER --signal 1
	echo "."
	;;
    force-reload)
	$0 reload
	;;
    restart)
	echo -n "Restarting VNC Server"
	start-stop-daemon --stop --quiet --retry 5 --oknodo --exec $VNC_SERVER --signal HUP
	start-stop-daemon --start --quiet --exec $VNC_SERVER
	echo "."
	;;
   *)
    echo "Usage: /etc/init.d/vncserver.sh {start|stop}"
    exit 1
esac

exit 0
