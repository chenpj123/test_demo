#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CFG_GPIO_DIR "/sys/class/gpio"
#define CFG_GPIO_SYS_FILE_EXPORTED_TIME_IN_100MS 10

int gpio_is_exported(size_t gpio)
{
	int fd = 0;
	int i;
	char buf[64] = {0};
	size_t len = 0;

	len = snprintf(buf, sizeof(buf), CFG_GPIO_DIR "/gpio%lu/direction", gpio);
	fd = open(buf, O_WRONLY);
	if (fd >= 0) {
		close(fd);
		return 1;
	}

	return 0;
}


int gpio_export(size_t gpio)
{
	int fd = 0;
	int i;
	char buf[64] = {0};
	size_t len = 0;

	if( gpio_is_exported(gpio))
	{
		return 0; //No need to re-export
	}

	fd = open(CFG_GPIO_DIR "/export", O_WRONLY);
	if( fd < 0 )
	{
		return -1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", gpio);
	write(fd, buf, len);
	close(fd);

	/* wait until file is actually available in user space */
	for (i = 0; i < CFG_GPIO_SYS_FILE_EXPORTED_TIME_IN_100MS; i++)
	{
		if( gpio_is_exported(gpio))
		{
			return 0; //GPIO is present in user space
		}
		usleep(100 * 1000);
	}

	return -1;
}

int gpio_unexport(size_t gpio)
{
	int fd = 0;
	char buf[64] = {0};
	size_t len = 0;

	fd = open(CFG_GPIO_DIR "/unexport", O_WRONLY);
	if( fd < 0 )
	{
		return -1;
	}

	len = snprintf(buf, sizeof(buf), "%lu", gpio);
	write(fd, buf, len);
	
	close(fd);
	return 0;
}

int gpio_set_direction(size_t gpio, int is_output)
{
	int fd = 0;
	char buf[64] = {0};
	size_t len = 0;

	len = snprintf(buf, sizeof(buf), CFG_GPIO_DIR "/gpio%lu/direction", gpio);

	fd = open(buf, O_WRONLY);
	if( fd < 0 )
	{
		return -1;
	}

	if(is_output)
	{
		write(fd, "out", 3);
	}
	else
	{
		write(fd, "in", 2);
	}

	close(fd);
	return 0;
}

int gpio_set_edge(size_t gpio, int rising, int falling)
{
	int fd = 0;
	char buf[64] = {0};
	size_t len = 0;

	len = snprintf(buf, sizeof(buf), CFG_GPIO_DIR "/gpio%lu/edge", gpio);

	fd = open(buf, O_WRONLY);
	if( fd < 0 )
	{
		return -1;
	}

	if(rising && falling)
	{
		write(fd, "both", 4);
	}
	else if(rising)
	{
		write(fd, "rising", 6);
	}
	else
	{
		write(fd, "falling", 7);
	}

	close(fd);
	return 0;
}

int gpio_set_value(size_t gpio, int value)
{
	int fd = 0;
	char buf[64] = {0};
	size_t len = 0;
	
	len = snprintf(buf, sizeof(buf), CFG_GPIO_DIR "/gpio%lu/value", gpio);

	fd = open(buf, O_WRONLY | O_NONBLOCK);
	if( fd < 0 )
	{
		return -1;
	}
	
	if (value != 0)
		write(fd, "1", 1);
	else
		write(fd, "0", 1);
		
	close(fd);
	return 0;	
}

int gpio_get_value(size_t gpio)
{
	int fd = 0;
	char buf[64] = {0};
	size_t len = 0;
	int ret, val;
	
	len = snprintf(buf, sizeof(buf), CFG_GPIO_DIR "/gpio%lu/value", gpio);

	fd = open(buf, O_RDONLY | O_NONBLOCK);
	if( fd < 0 )
	{
		return -1;
	}
	
	ret = read(fd, buf, 1);
	if (ret < 0)
	{
		close(fd);
		return -1;
	} 
	
	val = buf[0] - '0';	
	close(fd);
	return val;	
}

