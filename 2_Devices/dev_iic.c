#include "dev_iic.h"
#include "errno.h"
#include <string.h>

#include "drv_iic.h"

static struct I2CDev* HeadDev = NULL;

void AllI2CDev_Regis(void)
{
    I2CDev_Creat();
}

struct I2CDev* I2CDev_Find(char* name)
{
    struct I2CDev* Dev = HeadDev;
    
    while(Dev != NULL)
    {
        if(strstr(Dev->name,  name))
        {
            return Dev;
        }
        Dev = Dev->next;
    }
    return NULL;    
}

int I2CDev_Insert(struct I2CDev* Dev)
{
    if(Dev == NULL)   return -ENAVAIL;
    
    if(HeadDev == NULL)
    {
        HeadDev = Dev;
    }
    else
    {
        Dev->next = HeadDev;
        HeadDev = Dev;
    }
    
    return ESUCCESS;      
}
