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

	if(msg->msg_name != NULL) {
		inet_ntop(AF_INET,
					&((struct sockaddr_in*)msg->msg_name)->sin_addr,
					str_addr,
					sizeof(struct sockaddr_in));
		port = ntohs(((struct sockaddr_in*)msg->msg_name)->sin_port);
//		std::cout << "UDP message received from: " <<
//				str_addr << ":" <<
//				port << std::endl;
	}

//	std::cout << "Ctrl len: " << msg->msg_controllen << std::endl;

	for (cmptr = CMSG_FIRSTHDR(msg);
			cmptr != NULL;
			cmptr = CMSG_NXTHDR(msg, cmptr))
	{
//		std::cout << "length:" << cmptr->cmsg_len <<
//				" level: " << cmptr->cmsg_level <<
//				" type: " << cmptr->cmsg_type << std::endl;
		if(cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_TOS) {
			ecnptr = (int*)CMSG_DATA(cmptr);
			received_ecn = *ecnptr;
//			std::cout << "ECN: " << (int)(received_ecn & INET_ECN_MASK) << std::endl;
		}
	}

//	std::cout << "Buffer size: " << msg_len << " ..."<< std::endl;
//	std::cout << "Buffer dump: ";
//	for(int i = 0; i < msg_len; ++i) {
//		std::cout << ((char*)msg->msg_iov[0].iov_base)[i];
//	}
//	std::cout << std::endl;
}

