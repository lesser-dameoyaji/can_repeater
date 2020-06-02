all:
	gcc -I include -lpthread -li2c -lwiringPi main.c rpt.c cansock.c command/commander.c command/config.c command/id.c command/echo.c command/thread.c command/exit.c command/csock.c command/interface.c command/send.c command/route.c command/reset.c command/count.c lcd.c menu/menu.c menu/title.c menu/counter.c menu/main_list.c menu/exit.c menu/save.c menu/load.c
