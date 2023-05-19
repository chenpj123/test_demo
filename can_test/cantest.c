#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libsocketcan.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>

#define DEBUG 1
#define MAX_DEVICE_NUM	2
#define CAN1_NAME		"CAN1"
#define CAN2_NAME   "CAN2"
#define MAX_CAN_FRAME_LENGTH	8

//#define DEBUG
#ifdef DEBUG
#define debug_output printf
#endif

typedef struct _can_port_info_t
{
	char devicename[10];
	int opened;
	int fd;
	
	struct can_frame frame_cache;
	int cache_data_index;

}can_port_info_t;


static can_port_info_t all_can_ports[MAX_DEVICE_NUM] = {
	{CAN1_NAME,0},
	{CAN2_NAME,0}
};

static char *can_get_devname(const char *can_name)
{
	if (!strcasecmp(can_name,CAN1_NAME))
	{
		return "can0";
	}		
	if (!strcasecmp(can_name,CAN2_NAME))
	{
		return "can1";
	}		
	return NULL;
}

#ifdef DEBUG
static void can_print_buf(unsigned char *buf,int len)
{
	int i;
	for(i=0;i<len;i++)
		debug_output("%02X ",buf[i]);
	debug_output("\n");
}
#endif

int can_open(const char *device,int bitrate,int timeout)
{
	can_port_info_t *can_port;
	int nRet;
	int i,can_fd;
	struct ifreq ifr;
	struct sockaddr_can addr;
	
	char *devname = NULL;
	for(i=0;i<MAX_DEVICE_NUM;i++)
	{
		if (!strcasecmp(device,all_can_ports[i].devicename))
		{
			if (all_can_ports[i].opened)
				return i;
			can_port = all_can_ports+i;
			can_fd = i;
			break;
		}
	}
	if (i == MAX_DEVICE_NUM)
		return -1;

	devname = can_get_devname(device);
	if (devname == NULL)
		return -1;

	//stop it first.
	if (can_do_stop(devname)<0)
	{
		return -1;
	}		
	
	//set bitrate
	if (can_set_bitrate(devname,bitrate)<0)
		return -1;
	
	//start can.
	if(can_do_start(devname)<0)
		return -1;
		
	//open can socket.
	can_port->fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (can_port->fd < 0) 
	{
		return -1;
	}

	addr.can_family = PF_CAN;
	strcpy(ifr.ifr_name, devname);
	if (ioctl(can_port->fd, SIOCGIFINDEX, &ifr)) 
	{
		return -1;
	}
	
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(can_port->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
	{
		return -1;
	}

	can_port->opened = 1;
	can_port->cache_data_index = -1;
	memset(&(can_port->frame_cache),0,sizeof(struct can_frame));
	return can_fd;
}


int can_close(int can_fd)
{
	can_port_info_t *can_port;
	if (can_fd >= MAX_DEVICE_NUM)
		return 0;
	can_port = all_can_ports+can_fd;
	close(can_port->fd);
	can_port->opened = 0;
	return 1;
}

int can_write(int can_fd,unsigned int frame_id,char *buf,int size)
{
	can_port_info_t *can_port;
	int i,j,ret;
	int pos;
	struct can_frame frame = {
		.can_id = 1,
	};
		
	if (can_fd >= MAX_DEVICE_NUM)
		return -1;
	can_port = all_can_ports+can_fd;

	frame.can_id =frame_id;
	
	i=0;

	while(1)
	{
		if ((unsigned long)i>=size)
			break;

		if (size-i<MAX_CAN_FRAME_LENGTH)
			frame.can_dlc = size-i;
		else
			frame.can_dlc = MAX_CAN_FRAME_LENGTH;
			
		pos = i;
		for (j = 0; j < frame.can_dlc; j++ )
		{
			frame.data[j] = buf[pos++];
		}
		
		ret = write(can_port->fd, &frame, sizeof(frame));
		
		if (ret<0)
			break;
		if (ret != sizeof(frame))
		{
			fprintf(stderr, "can_write can not write full frame(full frame length:%d, real write length:%d)\n", sizeof(frame), ret);
			break;
		}
		i = pos;
#ifdef DEBUG		
		debug_output("can write: length=%d, packet: ",frame.can_dlc);
		can_print_buf(frame.data,frame.can_dlc);
#endif
	}
	return i;
}

static int can_read_internal(int can_fd,unsigned int *frame_id,char *buf,int size)
{
	can_port_info_t *can_port;
	struct can_frame frame;
	int i,nbytes;
	
	if (can_fd >= MAX_DEVICE_NUM)
		return -1;
	can_port = all_can_ports+can_fd;

	if (can_port->cache_data_index >=0)		//has data in cache.
	{
		int data_len = can_port->frame_cache.can_dlc-can_port->cache_data_index;
		*frame_id = can_port->frame_cache.can_id;
		if (data_len >= (int)size)
		{
			for(i=0;i<(int)size;i++)
				buf[i] = can_port->frame_cache.data[can_port->cache_data_index+i];
			can_port->cache_data_index += size;
			if (can_port->cache_data_index >= can_port->frame_cache.can_dlc)
				can_port->cache_data_index = -1;
			return size;
		}
		else
		{
			//read all cache.
			for(i=0;i<data_len;i++)
				buf[i] = can_port->frame_cache.data[can_port->cache_data_index+i];
			can_port->cache_data_index = -1;
			return data_len;
		}
	}

	nbytes = read(can_port->fd, &frame, sizeof(struct can_frame));
	if (nbytes > 0)
	{  
		*frame_id = frame.can_id;
		if (frame.can_id & CAN_RTR_FLAG)
		{
			return 0;
		}
		if ((int)size < frame.can_dlc)
		{
			for (i = 0; i < (int)size; i++)                           
			{
					buf[i] = frame.data[i];
			}
			memcpy(&(can_port->frame_cache),&frame,sizeof(struct can_frame));
			can_port->cache_data_index = size;
			return size;
		}
		else
		{
			for (i = 0; i < frame.can_dlc; i++)                           //Package receiving ok
			{
				buf[i] = frame.data[i];
			}
			return frame.can_dlc;
		}
	}

	return 0;
}

int can_read(int can_fd,unsigned int *frame_id,char *buf,int size)
{
	int ret=can_read_internal(can_fd,frame_id,buf,size);
#ifdef DEBUG
	if (ret > 0)
	{
		debug_output("can read ,length=%d,frame id=%d,packet: ",ret,*frame_id);
		can_print_buf(buf,ret);
	}
	else
		fprintf(stderr, "can read error:%d\n", ret);
#endif
	return ret;
}

int can_poll(int can_fd,int timeout)
{
	can_port_info_t *can_port;
	fd_set rfds;
	struct timeval tv;
	int sel_res;
	int fd;
	
	if (can_fd >= MAX_DEVICE_NUM)
		return -1;
	can_port = all_can_ports+can_fd;

	if (can_port->cache_data_index>=0)
		return 1;

	fd = can_port->fd;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout % 1000)*1000;
	return select(fd + 1, &rfds, NULL,NULL,&tv);
}



int main(int argc, char *argv[])
{
	int fd;
	unsigned char send_buffer[100];
	unsigned char recv_buffer[100];
	unsigned int frameid = 0x1;
	int i;

	if (argc < 2)
	{
		fprintf(stderr, "usage  : cantest canname(canname must be CAN1 or CAN2)\n");
		return -1;
	}

	fd = can_open(argv[1],125000,500);
	if (fd < 0)
	{
		printf("can't open %s\n", argv[1]);
		return -1;
	}
	
	for(i=0;i<20;i++)
	{
		send_buffer[i] = i;
	}
	
	while(1)
	{
		can_write(fd,frameid,send_buffer,20);
		//read.
		if (can_poll(fd,500)>0)
		{
			can_read(fd,&frameid,recv_buffer,8);
		}
	}
	
	return 0;
}

