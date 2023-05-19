#include <fcntl.h>      /* File control definitions */
#include <stdio.h>      /* Standard input/output */
#include <string.h>
#include <stdlib.h>
#include <termio.h>     /* POSIX terminal control definitions */
#include <sys/time.h>   /* Time structures for select() */
#include <unistd.h>     /* POSIX Symbolic Constants */
#include <assert.h>
#include <errno.h>      /* Error definitions */
#include <sys/mman.h>
#include "gpio.h"

#define GPIO_TO_NUM(bank, gpio) (32 * ((bank) - 1) + (gpio))
#define LED_GPIO_NUM	GPIO_TO_NUM(4, 16)

static int set_led(int is_on)
{
	int ret;

	ret = gpio_set_value(LED_GPIO_NUM, is_on);
	if (ret < 0)
	{
		fprintf(stderr, "gpio : %d set value(%d) failed!!!!\n", LED_GPIO_NUM, is_on);
		return -1;
	}
	else
		return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	int i;

        ret = gpio_export(LED_GPIO_NUM);
        if (ret < 0)
        {
                fprintf(stderr, "export gpio : %d failed!!!!\n", LED_GPIO_NUM);
                return -1;
        }

        ret = gpio_set_direction(LED_GPIO_NUM, 1);
        if (ret < 0)
        {
                fprintf(stderr, "gpio : %d set direction(output) failed!!!!\n", LED_GPIO_NUM);
                return -1;
        }

	for (i = 0; i < 10; i++)
	{
		set_led(1);
		sleep(1);
		set_led(0);
		sleep(1);
	}
	
	return 0;
}

