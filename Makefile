all:
	gcc -lpthread -li2c -lwiringPi main.c cansock.c uds.c lcd.c menu.c config.c filter.c


