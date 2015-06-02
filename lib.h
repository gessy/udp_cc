#ifndef LIB_HH_
#define LIB_HH_

#define MAX_BUF_SIZE 65535
#define MAX_CTRL_SIZE 8192
#define SERVER_PORT 9001

#define INET_ECN_NOT_ECT	0x00	/* ECN was not enabled */
#define INET_ECN_ECT_1		0x01	/* ECN capable packet */
#define INET_ECN_ECT_0		0x02	/* ECN capable packet */
#define INET_ECN_CE			0x03	/* ECN congestion */
#define INET_ECN_MASK		0x03	/* Mask of ECN bits */

void display_msg(struct msghdr *msg, int msg_len);

#endif /* LIB_HH_ */
