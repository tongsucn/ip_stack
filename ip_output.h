#ifndef IP_OUTPUT_H
#define IP_OUTPUT_H

#include "data_types.h"
#include "ip.h"

typedef struct ip_frag_list
{
	ip_fragment_t		*frag;
	struct ip_frag_list	*next;
} ip_frag_list_t;

typedef struct send_data
{
	long length;
	void *data;
} send_data_t;

int ip_send(int length, __u8 protocol, __u32 dest_addr, void *data);

ip_frag_list_t *ip_frag(int length, __u8 ttl, __u8 protocol, __u32 dest_addr, void *data);

__u16 get_id();

int reset_id(__u16 uid);

int ip_send_checksum(ip_fragment_t *frag);

void free_frag_list(ip_frag_list_t *frag_list);

int ip_forward(ip_fragment_t *frag);

int ip_output(__u32 dest_addr, ip_fragment_t *frag);

#endif // IP_OUTPUT_H
