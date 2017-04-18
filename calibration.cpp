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

#include "calibration.h"

int setCalibrationMatrix( point * displayPtr,
                          point * screenPtr,
                          calibMatrix * matrixPtr)
{

    int  retValue = OK ;



    matrixPtr->Divider = (((int64_t)screenPtr[0].x - (int64_t)screenPtr[2].x) * ((int64_t)screenPtr[1].y - (int64_t)screenPtr[2].y)) -
                         (((int64_t)screenPtr[1].x - (int64_t)screenPtr[2].x) * ((int64_t)screenPtr[0].y - (int64_t)screenPtr[2].y)) ;

    if( matrixPtr->Divider == 0 )
    {
        retValue = NOT_OK ;
    }
    else
    {
        matrixPtr->An = (((int64_t)displayPtr[0].x - (int64_t)displayPtr[2].x) * ((int64_t)screenPtr[1].y - (int64_t)screenPtr[2].y)) -
                        (((int64_t)displayPtr[1].x - (int64_t)displayPtr[2].x) * ((int64_t)screenPtr[0].y - (int64_t)screenPtr[2].y)) ;

        matrixPtr->Bn = (((int64_t)screenPtr[0].x - (int64_t)screenPtr[2].x) * ((int64_t)displayPtr[1].x - (int64_t)displayPtr[2].x)) -
                        (((int64_t)displayPtr[0].x - (int64_t)displayPtr[2].x) * ((int64_t)screenPtr[1].x - (int64_t)screenPtr[2].x)) ;

        matrixPtr->Cn = ((int64_t)screenPtr[2].x * (int64_t)displayPtr[1].x - (int64_t)screenPtr[1].x * (int64_t)displayPtr[2].x) * (int64_t)screenPtr[0].y +
                        ((int64_t)screenPtr[0].x * (int64_t)displayPtr[2].x - (int64_t)screenPtr[2].x * (int64_t)displayPtr[0].x) * (int64_t)screenPtr[1].y +
                        ((int64_t)screenPtr[1].x * (int64_t)displayPtr[0].x - (int64_t)screenPtr[0].x * (int64_t)displayPtr[1].x) * (int64_t)screenPtr[2].y ;

        matrixPtr->Dn = (((int64_t)displayPtr[0].y - (int64_t)displayPtr[2].y) * ((int64_t)screenPtr[1].y - (int64_t)screenPtr[2].y)) -
                        (((int64_t)displayPtr[1].y - (int64_t)displayPtr[2].y) * ((int64_t)screenPtr[0].y - (int64_t)screenPtr[2].y)) ;

        matrixPtr->En = (((int64_t)screenPtr[0].x - (int64_t)screenPtr[2].x) * ((int64_t)displayPtr[1].y - (int64_t)displayPtr[2].y)) -
                        (((int64_t)displayPtr[0].y - (int64_t)displayPtr[2].y) * ((int64_t)screenPtr[1].x - (int64_t)screenPtr[2].x)) ;

        matrixPtr->Fn = ((int64_t)screenPtr[2].x * (int64_t)displayPtr[1].y - (int64_t)screenPtr[1].x * (int64_t)displayPtr[2].y) * (int64_t)screenPtr[0].y +
                        ((int64_t)screenPtr[0].x * (int64_t)displayPtr[2].y - (int64_t)screenPtr[2].x * (int64_t)displayPtr[0].y) * (int64_t)screenPtr[1].y +
                        ((int64_t)screenPtr[1].x * (int64_t)displayPtr[0].y - (int64_t)screenPtr[0].x * (int64_t)displayPtr[1].y) * (int64_t)screenPtr[2].y ;
    }

    return( retValue ) ;

}

int getDisplayPoint( point * displayPtr,
                     point * screenPtr,
                     calibMatrix * matrixPtr )
{
    int  retValue = OK ;


    if( matrixPtr->Divider != 0 )
    {
        displayPtr->x = ( (matrixPtr->An * (int64_t)screenPtr->x) +
                          (matrixPtr->Bn * (int64_t)screenPtr->y) +
                           matrixPtr->Cn
                        ) / matrixPtr->Divider ;

        displayPtr->y = ( (matrixPtr->Dn * (int64_t)screenPtr->x) +
                          (matrixPtr->En * (int64_t)screenPtr->y) +
                           matrixPtr->Fn
                        ) / matrixPtr->Divider ;
    }
    else
    {
        retValue = NOT_OK ;
    }

    return( retValue ) ;

}
