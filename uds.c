#include "uds.h"

int uds_open_svr(char* pathname)
{
	struct sockaddr_un svr_addr;
	int fd;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd < 0)
	{
		printf("uds_open_svr: server socket error %s\n", pathname);
		return -1;
	}

	unlink(pathname);

	memset(&svr_addr, 0, sizeof(struct sockaddr_un));
	svr_addr.sun_family = AF_UNIX;
	strcpy(svr_addr.sun_path, pathname);
	
	if(bind(fd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr_un)) < 0)
	{
		printf("uds_open_svr: bind error %d\n", fd);
		close(fd);
		return  -1;
	}

	if(listen(fd, 5) < 0)
	{
		printf("listen error %d\n", fd);
		close(fd);
		return -1;
	}
	return fd;
}

int uds_open_cli(char* pathname)
{
	struct sockaddr_un svr_addr;
	int fd;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd < 0)
	{
		printf("client socket error %d\n", fd);
		return -1;
	}

	memset(&svr_addr, 0, sizeof(struct sockaddr_un));
	svr_addr.sun_family = AF_UNIX;
	strcpy(svr_addr.sun_path, pathname);

	if(connect(fd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr_un))  < 0)
	{
		printf("connect error %d\n", fd);
		return -1;
	}
	
	return fd;
}

void uds_close(int fd)
{
	close(fd);
}

