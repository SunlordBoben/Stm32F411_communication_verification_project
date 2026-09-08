#ifndef __DEV_IIC_H__
#define __DEV_IIC_H__

#include "stm32f4xx_hal.h"

typedef struct I2CDev{
    char *name;
    unsigned char channel;
    unsigned short own_addr;
    unsigned short slave_addr;
    
    int (*Init)(struct I2CDev *ptdev);
    int (*Write)(struct I2CDev *ptdev, unsigned char *buf, unsigned int length);
    int (*Read)(struct I2CDev *ptdev, unsigned char *buf, unsigned int length);
    struct I2CDev *next;
}I2CDevice;

void AllI2CDev_Regis(void);
struct I2CDev* I2CDev_Find(char* name);
int I2CDev_Insert(struct I2CDev* Dev);



#endif
