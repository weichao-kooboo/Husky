//
//  hky_crc32.h
//  Husky
//
//  Created by weichao on 5/31/18.
//  Copyright © 2018 weichao. All rights reserved.
//

#ifndef hky_crc32_h
#define hky_crc32_h

#include "hky_config.h"
#include "hky_core.h"

extern uint32_t hky_crc32_table256[];

static hky_inline uint32_t
hky_crc32_long(hky_uchar *p,size_t len){
    uint32_t crc;
    crc=0xffffffff;
    
    while (len--) {
        crc=hky_crc32_table256[(crc^*p++)&0xff]^(crc>>8);
    }
    return crc^0xffffffff;
}

#endif /* hky_crc32_h */
