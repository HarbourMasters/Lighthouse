
#ifndef BANJO_KAZOOIE_BOOL_H
#define BANJO_KAZOOIE_BOOL_H

    #ifdef LIGHTHOUSE_P
        #include <ultra64.h>

        #ifndef NOT
        #define NOT(boolean) ((boolean) ^ 1)
        #endif
        
        #ifndef BOOL
        #define BOOL(boolean) ((boolean) ? TRUE : FALSE)
        #endif

        #ifndef NULL
        #define NULL			0
        #endif

        #ifndef TRUE
        #define TRUE			1
        #endif

        #ifndef FALSE
        #define FALSE			0
        #endif

    #else
        #include <ultra64.h>

        typedef int bool;
        #define NOT(boolean) ((boolean) ^ 1)
        #define BOOL(boolean) ((boolean) ? TRUE : FALSE)

    #endif

#endif
