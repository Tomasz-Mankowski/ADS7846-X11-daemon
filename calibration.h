/*
 *
 *   Copyright (c) 2001, Carlos E. Vidales. All rights reserved.
 *
 *   This sample program was written and put in the public domain
 *    by Carlos E. Vidales.  The program is provided "as is"
 *    without warranty of any kind, either expressed or implied.
 *   If you choose to use the program within your own products
 *    you do so at your own risk, and assume the responsibility
 *    for servicing, repairing or correcting the program should
 *    it prove defective in any manner.
 *   You may copy and distribute the program's source code in any
 *    medium, provided that you also include in each copy an
 *    appropriate copyright notice and disclaimer of warranty.
 *   You may also modify this program and distribute copies of
 *    it provided that you include prominent notices stating
 *    that you changed the file(s) and the date of any change,
 *    and that you do not charge any royalties or licenses for
 *    its use.
 *
 *  Modification by Tomasz Mankowski 2017
 */

#ifndef _CALIBRATE_H_
#define _CALIBRATE_H_

#include <math.h>
#include <stdint.h>

#ifndef		OK
    #define		OK		        0
    #define		NOT_OK		   -1
#endif

struct point 
{
	int32_t x;
    int32_t y;
};

struct calibMatrix 
{
	int64_t    An,     /* A = An/Divider */
               Bn,     /* B = Bn/Divider */
               Cn,     /* C = Cn/Divider */
               Dn,     /* D = Dn/Divider */
               En,     /* E = En/Divider */
               Fn,     /* F = Fn/Divider */
               Divider;
};

int setCalibrationMatrix( point * display,
                                 point * screen,
                                 calibMatrix * matrix) ;

int getDisplayPoint( point * display,
                            point * screen,
                            calibMatrix * matrix ) ;


#endif  /* _CALIBRATE_H_ */
