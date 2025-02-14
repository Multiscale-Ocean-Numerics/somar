#ifdef CHF_LANG_CC
/*******************************************************************************
 *  SOMAR - Stratified Ocean Model with Adaptive Refinement
 *  Developed by Ed Santilli & Alberto Scotti
 *  Copyright (C) 2024 Thomas Jefferson University and Arizona State University
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 *  For up-to-date contact information, please visit the repository homepage,
 *  https://github.com/MUON-CFD/SOMAR.
 ******************************************************************************/

// This file contains macros to make fortran coding easier and more readable.
#endif //CHF_LANG_CC

#ifndef __AddlFortranMacros_H__INCLUDED__
#define __AddlFortranMacros_H__INCLUDED__
#ifndef CHF_LANG_CC


#if CH_SPACEDIM == 2
#  define HORIZIX(i,j,k)   i,0
#  define VERTIX(i,j,k)    0,j

#  define HORIZDOX(bx,i,j,k) do i = i##bx##lo0, i##bx##hi0
#  define HORIZDOY(bx,i,j,k)
#  define VERTDO(bx,i,j,k)   do j = i##bx##lo1, i##bx##hi1

#elif CH_SPACEDIM == 3
#  define HORIZIX(i,j,k)   i,j,0
#  define VERTIX(i,j,k)    0,0,k

#  define HORIZDOX(bx,i,j,k) do i = i##bx##lo0, i##bx##hi0
#  define HORIZDOY(bx,i,j,k) do j = i##bx##lo1, i##bx##hi1
#  define VERTDO(bx,i,j,k)   do k = i##bx##lo2, i##bx##hi2


#else
#  error Bad SpaceDim
#endif

#if CH_SPACEDIM == 2
#  define OFFSET0(i)       i##0            , i##1
#  define OFFSET1(i,o1)    i##0 o1##0      , i##1 o1##1
#  define OFFSET2(i,o1,o2) i##0 o1##0 o2##0, i##1 o1##1 o2##1
#elif CH_SPACEDIM == 3
#  define OFFSET0(i)       i##0            , i##1            , i##2
#  define OFFSET1(i,o1)    i##0 o1##0      , i##1 o1##1      , i##2 o1##2
#  define OFFSET2(i,o1,o2) i##0 o1##0 o2##0, i##1 o1##1 o2##1, i##2 o1##2 o2##2
#else
#  error Bad SpaceDim
#endif

#if CH_SPACEDIM == 2
#  define AVG1IX(f,i,ii)     (0.500d0)*( f(i0,i1) + f(i0+ii0,i1+ii1) )
#  define FAVG1IX0(f,i)      (0.500d0)*( f(i0  ,i1  ) + f(i0+1,i1  ) )
#  define BAVG1IX0(f,i)      (0.500d0)*( f(i0  ,i1  ) + f(i0-1,i1  ) )
#  define FAVG1IX1(f,i)      (0.500d0)*( f(i0  ,i1  ) + f(i0  ,i1+1) )
#  define BAVG1IX1(f,i)      (0.500d0)*( f(i0  ,i1  ) + f(i0  ,i1-1) )

#  define AVG2IX(f,i,ii,jj)  (0.250d0)*( f(i0        ,i1        ) + f(i0+ii0    ,i1+ii1    ) \
                                        +f(i0    +jj0,i1    +jj1) + f(i0+ii0+jj0,i1+ii1+jj1) )
#  define FFAVG2IX(f,i)      (0.250d0)*( f(i0  ,i1  ) + f(i0+1,i1  ) \
                                        +f(i0  ,i1+1) + f(i0+1,i1+1) )
#  define FBAVG2IX(f,i)      (0.250d0)*( f(i0  ,i1  ) + f(i0+1,i1  ) \
                                        +f(i0  ,i1-1) + f(i0+1,i1-1) )
#  define BFAVG2IX(f,i)      (0.250d0)*( f(i0  ,i1  ) + f(i0-1,i1  ) \
                                        +f(i0  ,i1+1) + f(i0-1,i1+1) )
#  define BBAVG2IX(f,i)      (0.250d0)*( f(i0  ,i1  ) + f(i0-1,i1  ) \
                                        +f(i0  ,i1-1) + f(i0-1,i1-1) )
#elif CH_SPACEDIM == 3
#  define AVG1IX(f,i,ii)     (0.500d0)*( f(i0    ,i1    ,i2    ) + f(i0+ii0,i1+ii1,i2+ii2) )
#  define FAVG1IX0(f,i)      (0.500d0)*( f(i0  ,i1  ,i2  ) + f(i0+1,i1  ,i2  ) )
#  define BAVG1IX0(f,i)      (0.500d0)*( f(i0  ,i1  ,i2  ) + f(i0-1,i1  ,i2  ) )
#  define FAVG1IX1(f,i)      (0.500d0)*( f(i0  ,i1  ,i2  ) + f(i0  ,i1+1,i2  ) )
#  define BAVG1IX1(f,i)      (0.500d0)*( f(i0  ,i1  ,i2  ) + f(i0  ,i1-1,i2  ) )
#  define FAVG1IX2(f,i)      (0.500d0)*( f(i0  ,i1  ,i2  ) + f(i0  ,i1  ,i2+1) )
#  define BAVG1IX2(f,i)      (0.500d0)*( f(i0  ,i1  ,i2  ) + f(i0  ,i1  ,i2-1) )

#  define AVG2IX(f,i,ii,jj)  (0.250d0)*( f(i0        ,i1        ,i2        ) + f(i0+ii0    ,i1+ii1    ,i2+ii2    ) \
                                        +f(i0    +jj0,i1    +jj1,i2    +jj2) + f(i0+ii0+jj0,i1+ii1+jj1,i2+ii2+jj2) )

#  define AVG3IX(f,i,ii,jj,kk)  (0.125d0)*( f(i0            ,i1            ,i2            ) + f(i0+ii0        ,i1+ii1        ,i2+ii2        ) \
                                           +f(i0    +jj0    ,i1    +jj1    ,i2    +jj2    ) + f(i0+ii0+jj0    ,i1+ii1+jj1    ,i2+ii2+jj2    ) \
                                           +f(i0        +kk0,i1        +kk1,i2        +kk2) + f(i0+ii0    +kk0,i1+ii1    +kk1,i2+ii2    +kk2) \
                                           +f(i0    +jj0+kk0,i1    +jj1+kk1,i2    +jj2+kk2) + f(i0+ii0+jj0+kk0,i1+ii1+kk1+kk1,i2+ii2+jj2+kk2) )
#else
#  Bad SpaceDim
#endif



#if 0
  ! CHECK_CONTAINS returns true if c contains e.
  ! c = container.
  ! cc = container centering (0 for CC, 1 for FC)
  ! e = CC element
  ! eg = CC element grow
#endif
#if CH_SPACEDIM == 2
#  define CHECK_CONTAINS(c, cc0, cc1, cc2, e, eg0, eg1, eg2)                                \
      (      (i##c##lo0 .le. i##e##lo0-(eg0)) .and. (i##e##hi0-(eg0) .le. i##c##hi0-(cc0))  \
       .and. (i##c##lo1 .le. i##e##lo1-(eg1)) .and. (i##e##hi1-(eg1) .le. i##c##hi1-(cc1)))

#elif CH_SPACEDIM == 3
#  define CHECK_CONTAINS(c, cc0, cc1, cc2, e, eg0, eg1, eg2)                                \
      (      (i##c##lo0 .le. i##e##lo0-(eg0)) .and. (i##e##hi0-(eg0) .le. i##c##hi0-(cc0))  \
       .and. (i##c##lo1 .le. i##e##lo1-(eg1)) .and. (i##e##hi1-(eg1) .le. i##c##hi1-(cc1))  \
       .and. (i##c##lo2 .le. i##e##lo2-(eg2)) .and. (i##e##hi2-(eg2) .le. i##c##hi2-(cc2)))

#endif


#if 0
  ! CHECK_CONTAINSFINE returns true if c contains e.
  ! c = coarse-level container.
  ! cc = container centering (0 for CC, 1 for FC)
  ! e = fine-level element
  ! eg = element grow
  ! r = refinement ratio vector between levels.
  !
  ! If c is face centered, then this is self-explanatory.

#endif
#if CH_SPACEDIM == 2
#  define CHECK_CONTAINSFINE(c, cc0, cc1, cc2, e, eg0, eg1, eg2, r)                                             \
      (      (i##c##lo0*(r(0)) .le. i##e##lo0-(eg0)) .and. (i##e##hi0-(eg0) .le. (i##c##hi0-(cc0)+1)*(r(0))-1)  \
       .and. (i##c##lo1*(r(1)) .le. i##e##lo1-(eg1)) .and. (i##e##hi1-(eg1) .le. (i##c##hi1-(cc1)+1)*(r(1))-1))

#elif CH_SPACEDIM == 3
#  define CHECK_CONTAINSFINE(c, cc0, cc1, cc2, e, eg0, eg1, eg2, r)                                             \
      (      (i##c##lo0*(r(0)) .le. i##e##lo0-(eg0)) .and. (i##e##hi0-(eg0) .le. (i##c##hi0-(cc0)+1)*(r(0))-1)  \
       .and. (i##c##lo1*(r(1)) .le. i##e##lo1-(eg1)) .and. (i##e##hi1-(eg1) .le. (i##c##hi1-(cc1)+1)*(r(1))-1)  \
       .and. (i##c##lo2*(r(2)) .le. i##e##lo2-(eg2)) .and. (i##e##hi2-(eg2) .le. (i##c##hi2-(cc2)+1)*(r(2))-1))

#endif



#endif //!CHF_LANG_CC
#endif //!__AddlFortranMacros_H__INCLUDED__
