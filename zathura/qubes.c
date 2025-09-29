#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <girara/log.h>

static int sockfd;

static const struct sockaddr_un addr = {
	.sun_family = AF_UNIX,	
	.sun_path = "/tmp/qubes_zath.sock",
};

int open_sock_con_qubes(void) {
	int con_res;
	const struct sockaddr_un *addrp = &addr;

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sockfd == -1) {
		girara_error(
			"Error: qubes sock failed to open: errno: %d", errno);
	}
	
	con_res = connect(sockfd, addrp, sizeof(addr));
	if (con_res == -1) {
		girara_error(
			"Error: qubes sock failed to connect: errno: %d", errno);
  }

	return 0;
}

int close_sock_con_qubes() {
	int res;
	res = shutdown(sockfd, SHUT_RDWR);
	if (res == -1) {
  	girara_error(
			"Error: qubes sock failed to shutdown: errno: %d", errno);
	}

	res = close(sockfd);
	if (res == -1) {
  	girara_error(
			"Error: qubes sockfd failed to close: errno: %d", errno);
	}
}

int send_bookname_qubes() {
		
}
