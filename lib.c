#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

#include "lib.h"

void display_msg(struct msghdr *msg, int msg_len)
{
	struct cmsghdr *cmptr;
	char str_addr[INET_ADDRSTRLEN];
	unsigned short port;
	int *ecnptr;
	unsigned char received_ecn;
	unsigned int i;

	if(msg->msg_name != NULL) {
		inet_ntop(AF_INET,
					&((struct sockaddr_in*)msg->msg_name)->sin_addr,
					str_addr,
					sizeof(struct sockaddr_in));
		port = ntohs(((struct sockaddr_in*)msg->msg_name)->sin_port);
		printf("UDP message received from: %s:%d\n", str_addr, port);
	}

	printf("Ctrl len: %d\n", (int)msg->msg_controllen);

	for (cmptr = CMSG_FIRSTHDR(msg);
			cmptr != NULL;
			cmptr = CMSG_NXTHDR(msg, cmptr))
	{
		printf("Length: %d, level: %d, type: %d\n",
				(int)cmptr->cmsg_len,
				(int)cmptr->cmsg_level,
				(int)cmptr->cmsg_type);
		if(cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_TOS) {
			ecnptr = (int*)CMSG_DATA(cmptr);
			received_ecn = *ecnptr;
			printf("ECN bits: %d\n", (int)(received_ecn & INET_ECN_MASK));
		}
	}

	printf("Msg len: %d \n", msg_len);

	printf("Buffer dump: ");
	for(i = 0; i < msg_len; ++i) {
		printf("%c", ((char*)msg->msg_iov[0].iov_base)[i]);
	}
	printf("\n");
}

