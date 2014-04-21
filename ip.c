#include "ip.h"

static __u32	local_addr = 0;

__u32 get_local_addr()
{
	return local_addr;
}

__u32 set_local_addr(__u32 addr)
{
	return local_addr = addr;
}

__u16 calc_checksum(const ip_fragment_t *frag)
{
	__u64 result = 0;
	__u16 *iter = (__u16 *)frag->head;
	__u16 head_length = frag->head->ihl * 2;

	for (; head_length > 0; head_length--, iter++)
		result += *iter;

	result = (result >> 16) + (result & 0xffff);
	result += (result >> 16);

	return ~result;
}
