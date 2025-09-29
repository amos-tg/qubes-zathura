#define HEADER_LEN_IN_BYTES 4
#define ZBOOK_READ_NOTIFY 6
#define BNBUF_LEN 2048

#include <stdint.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <girara/log.h>

// i'm using a lot of global variables because 
// of my lack of familiarity with the codebase
// and my need to get this done quickly and with
// minimal changes to zathura itself.

static unsigned char bnbuf[2048];

static int sockfd;

static const struct sockaddr_un addr = {
	.sun_family = AF_UNIX,	
	.sun_path = "/tmp/qubes_zath.sock",
};

int open_sock_con_qubes(void);
int close_sock_con_qubes(void);
static void init_bnbuf(void);

int send_bookname_qubes(unsigned char *bname);
static void set_bnbuf_header(uint32_t msg_len);

int open_sock_con_qubes(void) {
	int con_res;
	const struct sockaddr_un *addrp = &addr;

	init_bnbuf();
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

int close_sock_con_qubes(void) {
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

int send_bookname_qubes(unsigned char *bname) {
	int res;

	if sizeof(*bname) > 

	//res = write(sockfd, &bnbuf, sizeof());
	if (res == -1) {
  	girara_error(
			"Error: qubes sockfd write failed: errno: %d", errno);
	}


}

// takes up the first 4 bytes of the bnbuf array
// with the length of the message. (we are using 
// SOCK_STREAM so message boundaries are not 
// preserved) 
// Least_Significant_Bit is the rightmost bit
static void set_bnbuf_header(uint32_t msg_len) {
	int shift_count; 
	unsigned char *bnbufp = bnbuf;

	// first iter = sc = 0, 8 bits , 0     3 < HEADER_LEN_IN_BYTES
	//              sc = 8, 16 bits , 1
	//              sc = 16, 24 bits , 2 
	//              sc = 24, 32 bits , 3
	for (; bnbufp < (bnbuf + HEADER_LEN_IN_BYTES); ++bnbufp, shift_count += 8) {
		*bnbufp |= (msg_len >> shift_count);
	}
}

static void init_bnbuf(void) {
	unsigned char *bnbufp = bnbuf;

	for (; bnbufp < (bnbuf + BNBUF_LEN); ++bnbufp) {
  	*bnbufp = 0;	
	}
} 
