#include "dev_uart.h"
#include "errno.h"
#include <string.h>


#include "drv_uart.h"

static struct UARTDev* HeadDev = NULL;


void AllUARTDev_Regis(void)
{
    UARTDev_Creat();
}

struct UARTDev* UARTDev_Find(char* name)
{
    struct UARTDev* Dev = HeadDev;
    
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

int UARTDev_Insert(struct UARTDev* Dev)
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





