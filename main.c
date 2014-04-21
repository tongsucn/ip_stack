#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/msg.h>

#include "ip.h"
#include "ip_route.h"
#include "ip_output.h"
#include "ip_input.h"
#include "protocol.h"

#define RCV_BUF_SIZE 540

int route_queue_id = -1, rcv_queue_id = -1;

void packet_rcv();
void write_to_file(int queue_id);

int main(int argc, char *argv[])
{
	if (!argv)
		exit(-1);
	else
	{
		init_route_table();

		if (strcmp("./sender", argv[0]) == 0)
		{
			printf("I'm sender.\n");
			set_local_addr(0x17171717);

			route_table_add(0x17171701, 0x17171701);
			route_table_add(0x38383838, 0x17171701);

			FILE *fp = fopen(argv[1], "r");
			fseek(fp, 0L, SEEK_END);
			int file_length = ftell(fp);
			void *data = malloc(file_length);
			fclose(fp);

			int filedes = open(argv[1], O_RDONLY);
			read(filedes, data, file_length);

			ip_send(file_length, UDP, 0x38383838, data);

			close(filedes);
			free(data);

			printf("Finish sending!\n");
		}
		else if (strcmp("./router", argv[0]) == 0)
		{
			route_queue_id = msgget(PUBLIC_MSG_QUEUE_ROUTE_KEY, IPC_CREAT);
			if (signal(SIGUSR1, packet_rcv) == SIG_ERR)
			{
				printf("Signal reg error!\n");
				exit(-1);
			}
			printf("I'm router, my PID: %d\n", getpid());
			set_local_addr(0x17171701);

			route_table_add(0x17171701, 0x17171701);
			route_table_add(0x17171717, 0x17171717);
			route_table_add(0x38383838, 0x38383838);

			printf("Ready, waiting for data.\n");
			pause();
		}
		else if (strcmp("./rcv", argv[0]) == 0)
		{
			rcv_queue_id = msgget(PUBLIC_MSG_QUEUE_RCV_KEY, IPC_CREAT);
			if (signal(SIGUSR1, packet_rcv) == SIG_ERR)
			{
				printf("Signal reg error!\n");
				exit(-1);
			}
			printf("I'm receiver, my PID: %d\n", getpid());
			set_local_addr(0x38383838);

			route_table_add(0x17171701, 0x17171701);
			route_table_add(0x17171717, 0x17171701);

			printf("Ready, waiting for data.\n");
			pause();
		}
		else
		{
			printf("There's something wrong with the program's name.\n");
			exit(-1);
		}
	}

	exit(0);
}

void packet_rcv()
{
	int queue_id = (route_queue_id == -1 ? rcv_queue_id : route_queue_id);
	void *data = malloc(sizeof(RCV_BUF_SIZE));
	msgrcv(queue_id, data, RCV_BUF_SIZE, 0, IPC_NOWAIT);
	char *new_data = (char *)data;
	int rcv_queue_id = ip_rcv(&new_data[sizeof(long)]);
	printf("rcv_queue_id: %d\n", rcv_queue_id);
	if (rcv_queue_id == -1)
		exit(-1);
	else if (rcv_queue_id > 0)
	{
		write_to_file(queue_id);
		printf("Task finished!\n");

		exit(0);
	}

	return;
}

void write_to_file(int queue_id)
{
	defrag_queue_t *iter = defrag_queue_table;
	while (iter != NULL && iter->queue_id != queue_id)
		iter = iter->y_next;

	if (!iter)
		exit(-1);
	else
	{
		int filedes = open("./rcv_output.txt", O_WRONLY | O_APPEND | O_CREAT);

		while (iter != NULL)
		{
			write(filedes, iter->frag->ip_data,
				  iter->frag->head->tot_len - iter->frag->head->ihl);
			iter = iter->x_next;
		}

		close(filedes);
	}
}
