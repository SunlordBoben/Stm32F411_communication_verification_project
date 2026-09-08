#ifndef __PRINTF_H
#define __PRINTF_H

#ifndef USE_PRINTF
#define USE_PRINTF  (1)
#endif /* USE_PRINTF */

#ifndef USE_DRV_LOG
#define USE_DRV_LOG  (0)
#endif /* USE_DRV_LOG */

#if USE_PRINTF
    #include <stdio.h>
    #define debugprintf(...)    printf(__VA_ARGS__)
#else 
    #define xprintf(...)
#endif /* USE_PRINTF */

#if USE_DRV_LOG
    #include <stdio.h>
    #define DRVLOG(...)    printf(__VA_ARGS__)
#else 
    #define DRVLOG(...)
#endif /* USE_PRINTF */

#endif /* __PRINTF_H */
