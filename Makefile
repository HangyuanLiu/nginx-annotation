
default:	build

clean:
	rm -rf Makefile objs

build:
	$(MAKE) -f objs/Makefile
	$(MAKE) -f objs/Makefile manpage

install:
	$(MAKE) -f objs/Makefile install

upgrade:
	/home/liu/nginx/sbin/nginx -t

	kill -USR2 `cat /home/liu/nginx/logs/nginx.pid`
	sleep 1
	test -f /home/liu/nginx/logs/nginx.pid.oldbin

	kill -QUIT `cat /home/liu/nginx/logs/nginx.pid.oldbin`
