#ifndef __GPIO_H
#define __GPIO_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C"{
#endif

int gpio_is_exported(size_t gpio);
int gpio_export(size_t gpio);
int gpio_unexport(size_t gpio);
int gpio_set_direction(size_t gpio, int is_output);
int gpio_set_edge(size_t gpio, int rising, int falling);
int gpio_set_value(size_t gpio, int value);
int gpio_get_value(size_t gpio);

#ifdef __cplusplus
}
#endif

#endif
