/* Declaration of IP interface */

#ifndef	IP_H_
#define	IP_H_

#include "data_types.h"

#define PUBLIC_MSG_QUEUE_ROUTE_KEY	232301
#define PUBLIC_MSG_QUEUE_RCV_KEY		565656

typedef struct iphdr
{
	__u8		version: 4,
			ihl: 4;
	__u8		tos;
	__u16	tot_len;
	__u16	id;
	__u16	frag_off;
	__u8		ttl;
	__u8		protocol;
	__u16	check;
	__u32	saddr;
	__u32	daddr; 
	/* Ignore the option field */
} iphdr_t;

typedef struct ip_fragment
{
	iphdr_t	*head;
	void	*ip_data;
} ip_fragment_t;

__u32 get_local_addr();
__u32 set_local_addr(__u32 addr);
__u16 calc_checksum(const ip_fragment_t *frag);

#endif
