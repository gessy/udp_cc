#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

#include "lib.h"

int main(int argc, char *argv[])
{
	int sock_fd = -1, flag, ret;
	struct sockaddr_in sock_addr4;
	struct pollfd sock_fds[1];
	struct msghdr rcv_msg;
	struct msghdr snd_msg;
	struct sockaddr_storage src_addr;
	struct sockaddr_storage dst_addr;
	struct iovec rcv_iov[1];
	struct iovec snd_iov[1];
	char rcv_buf[MAX_BUF_SIZE];
	char snd_buf[MAX_BUF_SIZE];
	char rcv_ctrl_data[MAX_CTRL_SIZE];
	socklen_t optlen;
	unsigned char set;
	unsigned int ecn;

	(void)argc;
	(void)argv;

	/* Create socket */
	sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock_fd == -1) {
		perror("socket()");
		return EXIT_FAILURE;
	}

	/* Set socket to reuse addresses */
	flag = 1;
	ret = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&flag, (socklen_t)sizeof(flag));
	if(ret == -1) {
		perror("setsockopt()");
		close(sock_fd);
		return EXIT_FAILURE;
	}

	/* Set up server address and port */
	memset(&sock_addr4, 0, sizeof(struct sockaddr_in));
	sock_addr4.sin_family = AF_INET;
	/* Server accept connection from any address */
	sock_addr4.sin_addr.s_addr = htonl(INADDR_ANY);
	/* Listen at port 9001 */
	sock_addr4.sin_port = htons(SERVER_PORT);

	/* Bind address and socket */
	ret = bind(sock_fd, (struct sockaddr*)&sock_addr4, sizeof(sock_addr4));
	if(ret == -1) {
		perror("bind()");
		close(sock_fd);
		return EXIT_FAILURE;
	}

	/* Receive ECN in message */
	set = 1;
	ret = setsockopt(sock_fd, IPPROTO_IP, IP_RECVTOS, &set, sizeof(set));
	if(ret == -1) {
		perror("setsockopt()");
		close(sock_fd);
		return EXIT_FAILURE;
	}

	/* Send ECN capable packets */
	ecn = INET_ECN_ECT_0;
	ret = setsockopt(sock_fd, IPPROTO_IP, IP_TOS, &ecn, sizeof(ecn));
	if(ret == -1) {
		perror("setsockopt()");
		close(sock_fd);
		return EXIT_FAILURE;
	}

	/* Add socket file descriptor to array */
	sock_fds[0].fd = sock_fd;
	sock_fds[0].events = POLLIN;

	printf("Waiting ...\n");

	/* Prepare message for sending */
	snd_iov[0].iov_base = snd_buf;
	snd_buf[0] = 'A';
	snd_iov[0].iov_len = 1;

	snd_msg.msg_name = &dst_addr;
	snd_msg.msg_namelen = sizeof(dst_addr);
	snd_msg.msg_iov = snd_iov;
	snd_msg.msg_iovlen = 1;
	snd_msg.msg_control = 0;
	snd_msg.msg_controllen = 0;

	/* Prepare message for receiving */
	rcv_iov[0].iov_base = rcv_buf;
	rcv_iov[0].iov_len = MAX_BUF_SIZE;

	rcv_msg.msg_name = &src_addr;
	rcv_msg.msg_namelen = sizeof(src_addr);
	rcv_msg.msg_iov = rcv_iov;
	rcv_msg.msg_iovlen = 1;
	memset(rcv_ctrl_data, 0, sizeof(rcv_ctrl_data));
	rcv_msg.msg_control = rcv_ctrl_data;
	rcv_msg.msg_controllen = sizeof(rcv_ctrl_data);

	/* Never ending loop */
	while(1) {
		ret = poll(sock_fds, 1, 1000);
		if(ret > 0) {
			ret = recvmsg(sock_fd, &rcv_msg, 0);
			if(ret == - 1) {
				perror("recvmsg()");
				close(sock_fd);
				return EXIT_FAILURE;
			}

			display_msg(&rcv_msg, ret);

			/* Try to get ECN bits from packet using getsockopt() */
			ret = getsockopt(sock_fd, IPPROTO_IP, IP_TOS, (void*)&ecn, &optlen);
			if(ret == -1) {
				perror("getsockopt()");
				close(sock_fd);
				return EXIT_FAILURE;
			}

			/* Set destination address:port according source address of client */
			((struct sockaddr_in*)snd_msg.msg_name)->sin_family =
					((struct sockaddr_in*)rcv_msg.msg_name)->sin_family;
			((struct sockaddr_in*)snd_msg.msg_name)->sin_addr =
					((struct sockaddr_in*)rcv_msg.msg_name)->sin_addr;
			((struct sockaddr_in*)snd_msg.msg_name)->sin_port =
					((struct sockaddr_in*)rcv_msg.msg_name)->sin_port;

			/* Send message */
			ret = sendmsg(sock_fd, &snd_msg, 0);
			if( ret == -1 ) {
				perror("sendmsg()");
				close(sock_fd);
				return EXIT_FAILURE;
			}
		} else if(ret == 0) {
			printf("Timeout ...\n");
		} else {
			perror("poll()");
			close(sock_fd);
			return EXIT_FAILURE;
		}
	}

	close(sock_fd);

	return EXIT_SUCCESS;
}
