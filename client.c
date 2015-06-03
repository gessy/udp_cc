#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "lib.h"

int main(int argc, char *argv[])
{
	int sock_fd = -1, ret;
	struct addrinfo hints, *result, *rp;
	struct sockaddr_in server_sock_addr4;
	struct msghdr rcv_msg;
	struct msghdr snd_msg;
	struct iovec rcv_iov[1];
	struct iovec snd_iov[1];
	char rcv_buf[MAX_BUF_SIZE];
	char snd_buf[MAX_BUF_SIZE];
	char str_addr[INET_ADDRSTRLEN];
	char rcv_ctrl_data[MAX_CTRL_SIZE];
	socklen_t optlen;
	unsigned char set;
	unsigned int ecn;

	/* Client program has to be run with server name as first argument */
	if(argc != 2) {
		printf("Syntax: %s server\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Initialize addrinfo structure ... */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;			/* Allow IPv4 */
	hints.ai_socktype = SOCK_DGRAM;		/* Allow datagram protocol */
	hints.ai_flags = 0;					/* No flags required */
	hints.ai_protocol = IPPROTO_UDP;	/* Allow UDP protocol only */

	/* To get IP of server */
	ret = getaddrinfo(argv[1], "9001", &hints, &result);
	if( ret != 0 ) {
		perror("getaddrinfo()");
		return EXIT_FAILURE;
	}

	/* Try to use addrinfo from getaddrinfo() */
	for(rp=result; rp!=NULL; rp=rp->ai_next) {
		sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if( sock_fd == -1) {
			perror("socket()");
			continue;
		} else {
			break;
		}
	}

	memset(&server_sock_addr4, 0, sizeof(struct sockaddr_in));
	server_sock_addr4.sin_family = AF_INET;
	server_sock_addr4.sin_addr.s_addr = ((struct sockaddr_in*)rp->ai_addr)->sin_addr.s_addr;
	server_sock_addr4.sin_port = htons(SERVER_PORT);
	inet_ntop(AF_INET, &(server_sock_addr4.sin_addr), str_addr, sizeof(str_addr));

	/* Try to "connect" to this address ... the client will be able to send and
	 * receive packets only from this address. */
	if(connect(sock_fd, (struct sockaddr *)&server_sock_addr4, sizeof(server_sock_addr4)) == -1) {
		perror("connect()");
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

	/* Prepare message for sending */
	snd_iov[0].iov_base = snd_buf;
	snd_buf[0] = 'a';
	snd_iov[0].iov_len = 1;

	snd_msg.msg_name = NULL;	/* Socket is connected */
	snd_msg.msg_namelen = 0;
	snd_msg.msg_iov = snd_iov;
	snd_msg.msg_iovlen = 1;
	snd_msg.msg_control = 0;
	snd_msg.msg_controllen = 0;

	/* Prepare message for receiving */
	rcv_iov[0].iov_base = rcv_buf;
	rcv_iov[0].iov_len = MAX_BUF_SIZE;

	rcv_msg.msg_name = NULL;	/* Socket is connected */
	rcv_msg.msg_namelen = 0;
	rcv_msg.msg_iov = rcv_iov;
	rcv_msg.msg_iovlen = 1;
	rcv_msg.msg_control = rcv_ctrl_data;
	rcv_msg.msg_controllen = MAX_CTRL_SIZE;

	if( sendmsg(sock_fd, &snd_msg, 0) == -1 ) {
		perror("sendmsg()");
		close(sock_fd);
		return EXIT_FAILURE;
	}

	ret = recvmsg(sock_fd, &rcv_msg, 0);
	if ( ret == -1) {
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

	close(sock_fd);
	return EXIT_SUCCESS;
}
