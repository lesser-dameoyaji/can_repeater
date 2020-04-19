all:
	gcc -I include -lpthread -li2c -lwiringPi main.c rpt.c cansock.c command/commander.c command/id.c command/echo.c command/thread.c command/exit.c command/csock.c command/send.c command/route.c command/reset.c command/count.c lcd.c menu/menu.c
