#define HEADER_LEN_IN_BYTES 4
#define BNBUF_LEN 2048

#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <girara/log.h>
#include "qubes.h"


static void init_bnbuf(void);
static int num_bytes_u8(unsigned char *src);
static void set_bnbuf_header(uint32_t msg_len);


static unsigned char read_notify_req[2] = { 54, 59 };
static unsigned char bnbuf[2048];

static int sockfd;
static const struct sockaddr_un addr = {
	.sun_family = AF_UNIX,	
	.sun_path = "/tmp/qubes_zath.sock",
};


// opens the qubes unix socket.
// returns -1 with failure, otherwise 0.
int open_sock_con_qubes(void) {
	int con_res;
	const struct sockaddr_un *addrp = &addr;

	init_bnbuf();
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sockfd == -1) {
		girara_error(
			"Error: qubes sock failed to open: errno: %d", errno);
		return -1;
	}
	
	con_res = connect(sockfd, (sockaddr *) addrp, sizeof(addr));
	if (con_res == -1) {
		girara_error(
			"Error: qubes sock failed to connect: errno: %d", errno);
		return -1;
  }

	return 0;
}

// shuts down and closes the qubes socket.
// returns -1 with failure, otherwise 0. 
int close_sock_con_qubes(void) {
	int res;
	res = shutdown(sockfd, SHUT_RDWR);
	if (res == -1) {
  	girara_error(
			"Error: qubes sock failed to shutdown: errno: %d", errno);
		return -1;
	}

	res = close(sockfd);
	if (res == -1) {
  	girara_error(
			"Error: qubes sockfd failed to close: errno: %d", errno);
		return -1;
	}

	return 0;
}

// takes a null terminated string to send over
// as a request to pull the book from the server
// returns -1 on failure, otherwise 0.
int send_bookname_qubes(unsigned char *bname) {
	int res, nb;
	unsigned char *bnbufp = bnbuf;
  size_t buf_cont_size = sizeof(bname) + sizeof(read_notify_req) - 1;
	// -1 for null terminator byte in bname 

	if (buf_cont_size > sizeof(bnbuf)) {
		girara_error(
			"Error: qubes bnbuf was not big enough for bname plus req seq");
		return -1;
	} 

	// the length of one bookname should not exceed the size of a uint32_t
	set_bnbuf_header((uint32_t) buf_cont_size);

	memcpy(bnbufp, read_notify_req, 2);
	bnbufp += 2;

	// I don't want to include the null terminator byte
	nb = (num_bytes_u8(bname) - 1);

  memcpy(bnbufp, bname, nb);
	bnbufp += nb;

	res = write(sockfd, bnbuf, (bnbufp - bnbuf));
	if (res == -1) {
  	girara_error(
			"Error: qubes sockfd write failed: errno: %d", errno);
		return -1;
	}

	res = read(sockfd, bnbuf, 1); 
	switch (res) { 
		case -1: 
      girara_error(
				"Error: qubes sockfd read error: errno: %d", errno);
			return -1;
		case 0:
			girara_error(
				"Error: qubes sockfd read returned EOF");
			return -1;
		case 1:
			if (1 == bnbuf[0]) {
				return 0;
			} else {
      	girara_error(
					"Error: qubes sockfd read was not RECV_SEQ");
				return -1;
			}
	}

	return 0;
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

// initializes bnbuf
static void init_bnbuf(void) {
	unsigned char *bnbufp = bnbuf;

	for (; bnbufp < (bnbuf + BNBUF_LEN); ++bnbufp) {
  	*bnbufp = 0;	
	}
} 

// returns the number of bytes in the 
// u8 array, including the null terminated byte.
static int num_bytes_u8(unsigned char *src) {
	unsigned char *initial = src;
	for (; *src != '\0'; ++src) {};
  return (src - initial);
}

// returns -1 if there are any 
// negative char values, otherwise 0.
// this is a check function.
int char_to_uchar(char *c_arr) {
	for (; *c_arr != '\0'; ++c_arr) {
  	if (0 > *c_arr) {
			return -1;
		}
	}

	return 0;
}
