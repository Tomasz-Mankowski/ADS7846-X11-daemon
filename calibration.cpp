/*
 *   Modification by Tomasz Mankowski 2017
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
*/

#include "calibration.h"

int calibration::setCalibrationMatrix( point * displayPtr, point * screenPtr)
{
    int  retValue = 1;

    matrix.Divider = (((int64_t)screenPtr[0].x() - (int64_t)screenPtr[2].x()) * ((int64_t)screenPtr[1].y() - (int64_t)screenPtr[2].y())) -
                     (((int64_t)screenPtr[1].x() - (int64_t)screenPtr[2].x()) * ((int64_t)screenPtr[0].y() - (int64_t)screenPtr[2].y())) ;

    if( matrix.Divider == 0 )
    {
        retValue = 0;
    }
    else
    {
        matrix.An = (((int64_t)displayPtr[0].x() - (int64_t)displayPtr[2].x()) * ((int64_t)screenPtr[1].y() - (int64_t)screenPtr[2].y())) -
                    (((int64_t)displayPtr[1].x() - (int64_t)displayPtr[2].x()) * ((int64_t)screenPtr[0].y() - (int64_t)screenPtr[2].y())) ;

        matrix.Bn = (((int64_t)screenPtr[0].x() - (int64_t)screenPtr[2].x()) * ((int64_t)displayPtr[1].x() - (int64_t)displayPtr[2].x())) -
                    (((int64_t)displayPtr[0].x() - (int64_t)displayPtr[2].x()) * ((int64_t)screenPtr[1].x() - (int64_t)screenPtr[2].x())) ;

        matrix.Cn = ((int64_t)screenPtr[2].x() * (int64_t)displayPtr[1].x() - (int64_t)screenPtr[1].x() * (int64_t)displayPtr[2].x()) * (int64_t)screenPtr[0].y() +
                    ((int64_t)screenPtr[0].x() * (int64_t)displayPtr[2].x() - (int64_t)screenPtr[2].x() * (int64_t)displayPtr[0].x()) * (int64_t)screenPtr[1].y() +
                    ((int64_t)screenPtr[1].x() * (int64_t)displayPtr[0].x() - (int64_t)screenPtr[0].x() * (int64_t)displayPtr[1].x()) * (int64_t)screenPtr[2].y() ;

        matrix.Dn = (((int64_t)displayPtr[0].y() - (int64_t)displayPtr[2].y()) * ((int64_t)screenPtr[1].y() - (int64_t)screenPtr[2].y())) -
                    (((int64_t)displayPtr[1].y() - (int64_t)displayPtr[2].y()) * ((int64_t)screenPtr[0].y() - (int64_t)screenPtr[2].y())) ;

        matrix.En = (((int64_t)screenPtr[0].x() - (int64_t)screenPtr[2].x()) * ((int64_t)displayPtr[1].y() - (int64_t)displayPtr[2].y())) -
                    (((int64_t)displayPtr[0].y() - (int64_t)displayPtr[2].y()) * ((int64_t)screenPtr[1].x() - (int64_t)screenPtr[2].x())) ;

        matrix.Fn = ((int64_t)screenPtr[2].x() * (int64_t)displayPtr[1].y() - (int64_t)screenPtr[1].x() * (int64_t)displayPtr[2].y()) * (int64_t)screenPtr[0].y() +
                    ((int64_t)screenPtr[0].x() * (int64_t)displayPtr[2].y() - (int64_t)screenPtr[2].x() * (int64_t)displayPtr[0].y()) * (int64_t)screenPtr[1].y() +
                    ((int64_t)screenPtr[1].x() * (int64_t)displayPtr[0].y() - (int64_t)screenPtr[0].x() * (int64_t)displayPtr[1].y()) * (int64_t)screenPtr[2].y() ;
    }

    return(retValue) ;
}

point calibration::getDisplayPoint(point &screenPtr)
{
    int32_t x, y;
	if( matrix.Divider != 0 )
    {
        x = ((matrix.An * (int64_t)screenPtr.x()) +
             (matrix.Bn * (int64_t)screenPtr.y()) +
              matrix.Cn
            ) / matrix.Divider ;

        y = ((matrix.Dn * (int64_t)screenPtr.x()) +
             (matrix.En * (int64_t)screenPtr.y()) +
              matrix.Fn
            ) / matrix.Divider ;				 
    }
    else
    {
        return point(0,0);
    }

    return point(x,y);
}
