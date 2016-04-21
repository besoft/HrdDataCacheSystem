/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: splines.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:52 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef SPLINES_HEADER
#define SPLINES_HEADER

#include "forarray.h"
#include "vectors.h"
#include <limits>

#ifndef max_val
#define max_val(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min_val
#define min_val(a,b)            (((a) < (b)) ? (a) : (b))
#endif


//solution for system of linear equation
template <class Type>
void Progon3 (ForArray<Type> &a,ForArray<Type> &b,ForArray<Type> &c,ForArray<Type> &d,ForArray<Type> &x,int n)
{
  ForArray<Type> w(n + 1, 0.0);
  ForArray<Type> s(n + 1, 0.0);
  ForArray<Type> t(n + 1, 0.0);
  ForArray<Type> u(n + 1, 0.0);
  ForArray<Type> v(n + 1, 0.0);

  Type z;
  int i1;

  u(1) = 0.0;
  v(1) = 0.0;
  w(1) = 1.0;

  for(int i = 1; i <= n; i++)//  i = 1,n
  {
    i1 = i + 1;
    z  = 1.0/(a(i) + c(i)*v(i));
    v(i1) = -b(i)*z;
    u(i1) = (-c(i)*u(i) + d(i))*z;
    w(i1) = - c(i)*w(i)*z;
  }

  s(n) = 1.0;
  t(n) = 0.0;
  for(int i = n-1; i>=1; i--)//do i = n-1,1, -1
  {
    s(i) = v(i+1)*s(i+1) + w(i+1);
    t(i) = v(i+1)*t(i+1) + u(i+1);
  }
  x(n) = (d(n) - b(n)*t(1) - c(n)*t(n-1))/(a(n) + b(n)*s(1) + c(n)*s(n-1));

  for(int i = 1; i <= n-1; i++)
    x(i) = s(i)*x(n) + t(i);

  return;
}

//solution for system of linear equation
template <class Type>
void Progon5(int n,ForArray<Type> &a,ForArray<Type> &b,ForArray<Type> &c,ForArray<Type> &d,ForArray<Type> &e,
             ForArray<Type> &g,ForArray<Type> &z)

{
  ForArray<Type> u(n + 2, 0.0);
  ForArray<Type> v(n + 2, 0.0);
  ForArray<Type> w(n + 2, 0.0);
  ForArray<Type> p(n + 2, 0.0);
  ForArray<Type> q(n + 2, 0.0);
  ForArray<Type> r(n + 2, 0.0);
  ForArray<Type> s(n + 2, 0.0);
  ForArray<Type> t(n + 2, 0.0);
  ForArray<Type> aa(n + 2, 0.0);
  ForArray<Type> dd(n + 2, 0.0);


  v(1) = 0.0;
  v(2) = 0.0;
  w(1) = 0.0;
  w(2) = 0.0;
  u(1) = 0.0;
  u(2) = 0.0;
  //c
  for(int i = 1; i <=n; i++)//  do i = 1,n
  {
    dd(i)  =  d(i) + e(i)*v(i);
    aa(i)  =  a(i) + dd(i)*v(i+1) +e(i)*w(i);
    u(i+2) = (g(i) - dd(i)*u(i+1) - e(i)*u(i))/aa(i);
    v(i+2) = -(b(i)+dd(i)*w(i+1))/aa(i);
    w(i+2) = -c(i)/aa(i);
  }
  //c
  p(1) = 0.0;
  q(2) = 0.0;
  p(2) = 1.0;
  q(1) = 1.0;
  //c
  for(int i = 1; i <= n; i++)//    do i = 1,n
  {
    p(i+2) = -(dd(i)*p(i+1) + e(i)*p(i))/aa(i);
    q(i+2) = -(dd(i)*q(i+1) + e(i)*q(i))/aa(i);
  }
  //        enddo
  //        c
  t(n-1) = 0.0;
  s(n-1) = 0.0;
  t(n)   = 0.0;
  r(n)   = 0.0;
  r(n-1) = 1.0;
  s(n)   = 1.0;
  //c
  for(int i = n - 2; i >= 1; i--)//      do i = n-2,1,-1
  {
    t(i) = v(i+2)*t(i+1) +w(i+2)*t(i+2) + u(i+2);
    s(i) = v(i+2)*s(i+1) +w(i+2)*s(i+2) + p(i+2);
    r(i) = v(i+2)*r(i+1) +w(i+2)*r(i+2) + q(i+2);
  }
  //enddo
  //c
  Type a11, a12, a21, a22, b1, b2;
  a11 =  1.0 - q(n+1) - w(n+1)*r(1);
  a12 = -(p(n+1) + v(n+1) + w(n+1)*s(1));
  a21 = -(v(n+2)*r(1) + w(n+2)*r(2) + q(n+2));
  a22 =  1. - p(n+2) - v(n+2)*s(1) - w(n+2)*s(2);
  b1  = w(n+1)*t(1) + u(n+1);
  b2  = v(n+2)*t(1) + w(n+2)*t(2) + u(n+2);
  z(n-1) = ( b1*a22 - b2*a12)/(a11*a22-a12*a21);
  z(n)   = (-b1*a21 + b2*a11)/(a11*a22-a12*a21);
  //          c
  for(int i = 1; i <= n - 2; i++)//        do i = 1,n-2
    z(i) = t(i) + s(i)*z(n) + r(i)*z(n-1);
  return;
}

//smoothing 1D spline
template <class Type>
void Smspline(int n,int ib,int ind,ForArray<Type> &x,ForArray<Type> &y,ForArray<Type> &rho,
              Type ax,Type bx,
              ForArray<Type> &aa,ForArray<Type> &zm,Type xx,Type &sp,Type &dsp,Type &d2sp)
{

  ForArray<Type> a(n, 0.0);
  ForArray<Type> b(n, 0.0);
  ForArray<Type> c(n, 0.0);
  ForArray<Type> d(n, 0.0);
  ForArray<Type> e(n, 0.0);
  ForArray<Type> g(n, 0.0);

  int nn = n;
  Type h1, h2, h3, h4,di;
  if (ind==0)
  {
    switch (ib)
    {
    case 1:
      h1 = x(2)-x(1);
      a(1)   = h1/3.0+(rho(1)+rho(2))/(h1*h1);
      h2     = x(3) - x(2);
      b(1)   = h1/6.0-(1./h1 +1./h2)*rho(2)/h1 - rho(1)/(h1*h1);
      c(1)   = rho(2)/(h1*h2);
      g(1)   = (y(2) - y(1))/h1 - ax;
      e(3)   = c(1);
      h1     = x(n) - x(n-1);
      h2     = x(n-1) - x(n-2);
      a(n)   = h1/3.0 +(rho(n-1)+rho(n))/(h1*h1);
      b(n-1) = h1/6.0-(1./h1 +1./h2)*rho(n-1)/h1-rho(n)/(h1*h1);
      c(n-2) = rho(n-1)/(h1*h2);
      e(1) = 0.;
      e(2) = 0.;
      d(1) = 0.;
      d(2) = b(1);
      c(n-1) = 0.;
      c(n) = 0.;
      b(n) = 0.;
      d(n) = b(n-1);
      g(n) = bx - (y(n) - y(n-1))/h1;
      e(n) = c(n-2);
      break;
    case 2:
      a(1)   = 1.0;
      a(n)   = 1.0;
      b(1)   = 0.;//!!!???  1.0
      b(n-1) = 0.;//!!!???  1.0
      c(1)   = 0.;
      c(n-2) = 0.;
      g(1)   = ax;// !!!!!!!!!!!!!!!!! 0.
      g(n)   = bx;// !!!!!!!!!!!!!!!!! 0.
      e(1)   = 0.;
      e(2)   = 0.;
      e(3)   = 0.;
      d(1)   = 0.;
      d(2) = b(1);//  !!!!!!!!!??????????????????

      b(n)   = 0.;
      c(n)   = 0.;
      c(n-1) = 0.;

      d(n)   = 0.;//  !!!!!!!!!!  SUKA, 14.08.2000 !!!!! 6 chasov iskal oshibku
      break;

    case 3:
      h1 = x(2) - x(1);//  !!! h(1)
      h2 = x(n) - x(n-1);//  !!! h(n-1)
      h3 = x(3) - x(2);//  !!! h(2)
      a(1) = (h1 + h2)/3.0 + rho(n-1)/(h2*h2) + rho(2)/(h1*h1) + (1.0/h1 + 1.0/h2)*(1.0/h1 + 1.0/h2)*rho(1);
      //*****         b(1) = h1/6.d0 -((1.d0/h2 + 1.d0/h1)*rho(1)+
      //*****     *                  (1.d0/h2 + 1.d0/h3)*rho(2))/h2
      b(1) = h1/6.0 -((1.0/h2 + 1.0/h1)*rho(1)+ (1.0/h1 + 1.0/h3)*rho(2))/h1;
      //*****         c(1) = rho(1)/(h1*h3)
      c(1) = rho(2)/(h1*h3);//  !!!?????????????????
      //!!!!!!!!!!!!

      //*****         g(1) = (y(2) - y(1))/h1 - (y(1) - y(n-1))/h2
      //*****
      g(1) = (y(2) - y(1))/h1 - (y(1) - y(n-1))/h2;
      //!!!!!!!!!!!!???          g(1) = (y(2) - y(1))/h1 - (y(n) - y(n-1))/h2
      d(2) = b(1);
      e(3) = c(1);
      h4   = x(n-1) - x(n-2);//  !!! h(n-2);
      //**************
      c(n-2) = rho(n-1)/(h2*h4);// !!!???????????
      //**************         d(n-2) = rho(n-1)/(h2*h4) !!! ShV 31.07.00
      c(n-1) = rho(n)/(h2*h1);
      //***???         b(n-1) = h2/3.d0 - ((1.d0/h4 + 1.d0/h2)*rho(n-1) +
      b(n-1) = h2/6.0 - ((1.0/h4 + 1.0/h2)*rho(n-1) + (1.0/h2 + 1.0/h1)*rho(n))/h2;
      //*****************
      d(1) =   h2/6.0 - ((1.0/h4 + 1.0/h2)*rho(n-1) + (1.0/h2 + 1.0/h1)*rho(1))/h2;
      //*****************
      e(1) = c(n-2);
      //*****************                          d(1) = h2/6.d0 !!!! ShV 30.07.00
      //*****************                          e(1) = 0 !!!! ShV 30.07.00
      //*****************                          c(1) = 0 !!!! ShV 30.07.00

      e(2) = c(n-1);
      g(n-1) = (y(n) - y(n-1))/h2 - (y(n-1) - y(n-2))/h4;
      nn = n - 1;
      break;
    default:
      break;
    }
    for(int i = 2; i <= n - 1; i++)//  do i = 2, n-1
    {
      h1   = x(i)   - x(i-1);
      h2   = x(i+1) - x(i);
      a(i) = (h1+h2)/3.0 + rho(i-1)/(h1*h1) + rho(i+1)/(h2*h2) + (1.0/h1 + 1.0/h2)*(1.0/h1 + 1.0/h2)*rho(i);
      g(i) = (y(i+1)-y(i))/h2 - (y(i)-y(i-1))/h1;
      if (i<=(n-2))
      {
        h3 = (x(i+2)-x(i+1));
        b(i) = h2/6.0 - ((1.0/h1 + 1.0/h2)*rho(i)+(1.0/h2 + 1.0/h3)*rho(i+1))/h2;
        d(i+1) = b(i);
      }
      if (i<=(n-3))
      {
        c(i) = rho(i+1)/(h2*h3);
        e(i+2) = c(i);
      }
    }
    Progon5(nn,a,b,c,d,e,g,zm);
    aa(1) = y(1) - rho(1)*(zm(2)-zm(1))/(x(2)-x(1));
    aa(n) = y(n) + rho(n)*(zm(n)-zm(n-1))/(x(n)-x(n-1));

    if (ib==3)
    {
      zm(n) = zm(1);
      di = (zm(2)-zm(1))/(x(2)-x(1));
      di = di - (zm(n)-zm(n-1))/(x(n)-x(n-1));
      aa(1) = y(1) - rho(1)*di;
      aa(n) = aa(1);
    }
    for(int i = 2; i <= n - 1; i++)// do i = 2, n-1
    {
      di = (zm(i+1)-zm(i))/(x(i+1)-x(i));
      di = di - (zm(i)-zm(i-1))/(x(i)-x(i-1));
      aa(i) = y(i) - rho(i)*di;
    }
  }
  else
  {
    int ni , i;
    Type h, tt;
    for(i = 2; i <= n; i++)// do  i = 2,n
    {
      ni = i;
      if(x(i)>xx - 0.1e-5)
        break;
    }
    i  = ni - 1;
    h  = x(i+1) - x(i);
    tt = (xx - x(i))/h;
    sp = aa(i)*(1.-tt) + aa(i+1)*tt - h*h/6.0*tt*(1.-tt)* ((2.0-tt)*zm(i) + (1.0+tt)*zm(i+1));
    dsp= (aa(i+1) - aa(i))/h - h/6.0*((2.0 - 6.0*tt + 3.0*tt*tt)*zm(i) +(1.0 - 3.0*tt*tt)*zm(i+1));
    d2sp = (1. - tt)*zm(i) + tt*zm(i+1);
  }
  return;
}

//interpolating 1D spline
template <class Type>
void Spline (int n,ForArray<Type> &x,ForArray<Type> &y,ForArray<Type> &z,
             int ind,int ib,Type ax,Type bx,Type xx,Type &sp,Type &dsp,Type &d2sp)
{
  ForArray<Type> a(n, 0.0);
  ForArray<Type> b(n, 0.0);
  ForArray<Type> c(n, 0.0);
  ForArray<Type> d(n, 0.0);

  int ip, ne, ns, nf;
  ip = 1;
  if (ind==0)
  {
    Type h1, h2, am, al, gn, g0, cc;
    a(1) =  2.0;
    ne   =  n;
    ns   =  2;
    nf   =  n - 1;
    switch (ib)//  !	  select case (ib)
    {
    case 1:
      b(1) = 0.0;
      c(1) =  0.0;
      d(1) =  2.0*ax;
      a(n) =  2.0;
      b(n) =  0.0;
      c(n) =  0.0;
      d(n) =  2.0*bx;
      break;
    case 2:
      b(1) =  1.0;
      c(1) =  0.;
      h1   =  x(2)-x(1);
      d(1) =  3.0*(y(2)-y(1))/h1 - 0.50*h1*ax;
      a(n) =  2.0;
      b(n) =  0.;
      c(n) =  1.0;
      h1   =  x(n)-x(n-1);
      d(n) =  3.0*(y(n)-y(n-1))/h1 + 0.50*h1*bx;
      break;
    case 3:
      h1   = x(2) - x(1);
      h2   = x(n) - x(n-1);
      am   = h2/(h1 + h2);
      al   = 1.0 - am;
      b(1) = am;
      c(1) = al;
      d(1) = 3.0*(am*(y(2) - y(1))/h1 +al*(y(1) - y(n-1))/h2);
      h1   = x(n-1) - x(n-2);
      h2   = x(n) - x(n-1);
      am   = h1/(h1 + h2);
      al   = 1.0 - am;
      a(n-1) = 2.0;
      b(n-1) = am;
      c(n-1) = al;
      d(n-1) = 3.0*(am*(y(n) - y(n-1))/h2 +al*(y(n-1) - y(n-2))/h1);
      nf = n - 2;
      ne = n - 1;
      break;
    case 4:
      h1   = x(2)-x(1);
      h2   = x(3)-x(2);
      g0   = h1/h2;
      a(2) = 1.0 + g0;
      b(2) = g0;
      c(2) = 0.;
      am   = h1/(h1+h2);
      al   = 1.0- am;
      cc   = am*(y(3)-y(2))/h2 + al*(y(2)-y(1))/h1;
      d(2) = cc + 2.0*g0*(y(3)-y(2))/h2;
      h2   = x(n)-x(n-1);
      h1   = x(n-1)-x(n-2);
      gn   = h1/h2;
      a(n-1) = 1.0 + gn;
      b(n-1) = 0.;
      c(n-1) = gn;
      am = h1/(h1+h2);
      al = 1.0 - am;
      cc = am*(y(n)-y(n-1))/h2 +al*(y(n-1)-y(n-2))/h1;
      d(n-1) = cc + 2.0*gn*(y(n-1)-y(n-2))/h1;
      ns = 3;
      nf = n - 2;
      ne = n - 2;
      ip = 2;
      break;
    default:
      break;
    }
    for(int j = ns; j <= nf; j++)//  do  j  = ns ,nf
    {
      h1   = x(j + 1) - x(j);
      h2  = x(j) - x(j-1);
      am   = h2/(h2 + h1);
      al   = 1.0 - am;
      c(j) = al;
      a(j) = 2.0;
      b(j) = am;
      d(j) = 3.0*(am*(y(j+1) - y(j))/h1 +al*(y(j) - y(j-1))/h2);
    }
    int aotmp = a.incOffset(ip);
    int botmp = b.incOffset(ip);
    int cotmp = c.incOffset(ip);
    int dotmp = d.incOffset(ip);
    int zotmp = z.incOffset(ip);
    Progon3 (a,b,c,d,z,ne);
    a.setOffset(aotmp);
    b.setOffset(botmp);
    c.setOffset(cotmp);
    d.setOffset(dotmp);
    z.setOffset(zotmp);

    switch (ib)
    {
    case 3:
      z(n) = z(1);
      break;
    case 4:
      z(1) = g0*g0*z(3)+(g0*g0-1.0)*z(2)+2.0*((y(2)-y(1))/(x(2)-x(1))-g0*g0*(y(3)-y(2))/(x(3)-x(2)));
      z(n) = gn*gn*z(n-2)+(gn*gn-1.0)*z(n-1)+2.0*((y(n)-y(n-1))/(x(n)-x(n-1))-gn*gn*(y(n-1)-y(n-2))/(x(n-1)-x(n-2)));
      break;
    default:
      break;
    }
  }
  else
  {
    int nj, j;
    Type h, tt, rp, aa, bb;
    for(j = 2; j <= n; j++)//do  j = 2,n
    {
      nj= j;
      //!!!!!!!!            if(x(j).gt.xx) go to 1
      if(x(j)>xx-0.1e-5) 
        break;
    }
    j  = nj - 1;
    h  = x(j+1) - x(j);
    tt = (xx - x(j))/h;
    rp = (y(j+1) - y(j))/h;
    aa = -2.0*rp + z(j) + z(j+1);
    bb = -aa +rp - z(j);
    //!!!!!!!!!!!!!	    bb = -aa +rp + z(j)
    sp = y(j) + (xx - x(j))*(z(j) + tt*(bb + tt*aa));
    dsp= z(j) + tt*(bb + aa*tt) + tt*(bb + 2.0*aa*tt);
    d2sp = (2.0*bb + 6.0*aa*tt)/h;
  }
  return;
}

//interpolating 2D spline
template <class Type>
void Splint2(int m,ForArray<Type> &x,int n,ForArray<Type> &y,ForArray<Type> &z,ForArray<Type> &zl,ForArray<Type> &zr,ForArray<Type> &zu,ForArray<Type> &zd,
             ForArray<Type> &zx,ForArray<Type> &zy, ForArray<Type> &zxy,int ind,int ibx,
             int iby,Type xx,Type yy,
             Type &spl)
{
  ForArray<Type> su(max_val(m, n) + 2,0);
  ForArray<Type> sd(max_val(m, n) + 2,0);
  ForArray<Type> zz(m * n+2,0);

  if (ind==0)
  {
    Type sp, dsp, d2sp;
    for(int j = 1; j <= n; j++)
    {
      int zotmp = z.incOffset(1+(j-1)*m);
      int zxotmp = zx.incOffset(1+(j-1)*m);
      Spline (m,x,z,zx,ind,ibx,zl(j),zr(j),xx,sp,dsp,d2sp);
      z.setOffset(zotmp);
      zx.setOffset(zxotmp);
    }
    if (iby<=2)
    {
      Spline (m,x,zd,sd,0,ibx,zxy(1),zxy(2),xx,sp,dsp,d2sp);
      Spline (m,x,zu,su,0,ibx,zxy(3),zxy(4),xx,sp,dsp,d2sp);
    }
    for(int i = 1; i <= m; i++)//      do i = 1,m
    {
      for(int j = 1; j <= n; j++)// do j = 1,n
      {
        zz(j) = zx(i + (j-1)*m);
      }
      int zxyotmp = zxy.incOffset(1+(i-1)*n);
      Spline (n,y,zz,zxy,0,iby,sd(i),su(i),xx,sp,dsp,d2sp);
      zxy.setOffset(zxyotmp);
    }
    for(int i = 1; i <= m; i++)//do i = 1,m
    {
      for(int j = 1; j <= n; j++) //do j = 1,n
      {
        zz(j) = z(i + (j-1)*m);
      }
      int zyotmp = zy.incOffset(1+(i-1)*n);
      Spline (n,y,zz,zy,0,iby,zd(i),zu(i),xx,sp,dsp,d2sp);
      zy.setOffset(zyotmp);
    }
  }
  else
  {
    int ni, nj, i, j;
    Type hx, hy, tx, ty;
    ForArray<Type> f(4), g(4), sum(4);

    for(i = 2; i <= m; i++) //do  i = 2,m
    {
      ni = i;
      if(x(i)>xx) 
        break;
    }
    i = ni - 1;
    for(j = 2; j <= n; j++)// do  j = 2,n
    {
      nj = j;
      if(y(j)>yy)
        break;
    }
    j = nj - 1;
    hx = x(i+1) - x(i);
    tx = (xx - x(i))/hx;
    hy = y(j+1) - y(j);
    ty = (yy - y(j))/hy;

    f(1) = (1.- tx)*(1.- tx)*(1. + 2.*tx);
    f(2) = tx*tx*(3.- 2*tx);
    f(3) = tx*(1. - tx)*(1. - tx)*hx;
    f(4) =-tx*tx*(1.- tx)*hx;

    g(1) = (1.- ty)*(1.- ty)*(1. + 2.*ty);
    g(2) = ty*ty*(3.- 2*ty);
    g(3) = ty*(1. - ty)*(1. - ty)*hy;
    g(4) =-ty*ty*(1.- ty)*hy;

    sum(1) =  z(i+(j-1)*m)*f(1) +  z(i+1+(j-1)*m)*f(2)+ zx(i+(j-1)*m)*f(3) + zx(i+1+(j-1)*m)*f(4);
    sum(2) =  z(i+j*m)*f(1) +  z(i+1+j*m)*f(2)+ zx(i+j*m)*f(3) + zx(i+1+j*m)*f(4);
    sum(3) =  zy(j+(i-1)*n)*f(1) +  zy(j+i*n)*f(2)+ zxy(j+(i-1)*n)*f(3) + zxy(j+i*n)*f(4);
    sum(4) =  zy(j+1+(i-1)*n)*f(1) +  zy(j+1+i*n)*f(2)+ zxy(j+1+(i-1)*n)*f(3) + zxy(j+1+i*n)*f(4);

    spl = 0.;
    for(int k = 1; k <= 4; k++)// do k  = 1, 4
      spl = spl + g(k)*sum(k);
  }
  return;
}

//smoothing 2D spline
template <class Type>
void Smspl2(int m,ForArray<Type> &x,int n,ForArray<Type> &y,ForArray<Type> &z,ForArray<Type> &zl,ForArray<Type> &zr,ForArray<Type> &zu,ForArray<Type> &zd,ForArray<Type> &rho,
            ForArray<Type> &sgm,ForArray<Type> &zx,ForArray<Type> &zy,ForArray<Type> &zxy,int ind,int ibx,int iby,Type xx,
            Type yy,
            Type &spl)
{


  ForArray<Type> zm(m * n,0);
  ForArray<Type> zz(m * n+2,0);

  if (ind==0)// then
  {
    //**********        if (ibx.eq.2) then   !!!!!!!!!!!!!   
    for(int i = 1; i <= m; i++)//do i=1,m
    {
      zu(i) = 0.;
      zd(i) = 0.;
    }
    for(int j = 1; j <= n; j++)//do j=1,n
    {
      zl(j) = 0.;
      zr(j) = 0.;
    }
    for(int j = 1; j <= 4; j++)//do j=1,4
    {
      zxy(j) = 0.;
    }
    //        **********         endif
    Type sp, dsp, d2sp;
    for(int j =1; j <= n; j++)// do j = 1,n
    {
      int zotmp = z.incOffset(1+(j-1)*m);
      int zxotmp = zx.incOffset(1+(j-1)*m);
      Smspline (m,ibx,0,x,z,rho,zl(j),zr(j),zx,zm,xx,sp,dsp,d2sp);
      z.setOffset(zotmp);
      zx.setOffset(zxotmp);
    }

    for(int i = 1; i <= m; i++)// do i = 1,m
    {
      //           ***       write (1,"(a,i5, 2f12.7)") 'first_zz',n, rho(i), sgm(i)
      for(int j = 1; j <= n; j++)// do j = 1,n
      {
        zz(j) = zx(i + (j-1)*m);
        //          ***             write (1,*)  zz(j)
      }
      int zyotmp = zy.incOffset(1+(i-1)*n);
      Smspline (n,iby,0,y,zz,sgm,zu(i),zd(i),zy,zm,xx,sp,dsp,d2sp);
      zy.setOffset(zyotmp);
    }

    for(int i = 1; i <= m; i++)//do i = 1,m  !!! MODIFICATION of Source DATA !!!
    {
      //              ***       write (1,"(a,i5, 2f12.7)") 'second_zy',n, rho(i), sgm(i)
      for(int j = 1; j <= n; j++)// do j = 1,n
      {
        z(i+(j-1)*m) = zy(j+ (i-1)*n);
        //***             write (1,*)  zy(j+ (i-1)*n)
      }
    }

    Splint2 (m,x,n,y,z,zl,zr,zu,zd,zx,zy,zxy,0,ibx,iby,xx,yy,spl);
  }
  else
  {
    Splint2 (m,x,n,y,z,zl,zr,zu,zd,zx,zy,zxy,1,ibx,iby,xx,yy,spl);
  }
  return;
}

//creation of regular grid based on initially given set of lines
template <class Type>
void produceRegularGrid(const std::vector<std::vector<V3d<Type> >* > &points, unsigned numpnts, Type rhoParam, unsigned splinetype, bool skiprearrange, std::vector<V3d<Type> > &result)
{
  V3d<Type> sp, dsp, d2sp, refpoint;
  refpoint = points[0]->at(0);

  result.assign(numpnts * points.size(), V3d<Type>(Type(0), Type(0), Type(0)));

  //resampling is done line by line
  for(unsigned i = 0; i < points.size(); i++)
  {
    //arrays to process separately all components of 3D vectors
    ForArray<Type> x[3];
    ForArray<Type> y[3];
    ForArray<Type> rho[3];

    ForArray<Type> zm[3];
    ForArray<Type> aa[3];

    //initial sizes
    for(int j = 0; j < 3; j++)
    {
      x[j].assign(points[i]->size(),Type(0));
      y[j].assign(points[i]->size(),Type(0));
      rho[j].assign(points[i]->size(),Type(0));

      zm[j].assign(points[i]->size(),Type(0));
      aa[j].assign(points[i]->size(),Type(0));
    }


    //filling with values from input parameters
    for(unsigned k = 0; k < points[i]->size(); k++)
    {
      for(int j = 0; j < 3; j++)
      {
        x[j][k] = k;
        y[j][k] = points[i]->at(k)[j];
        rho[j][k] = rhoParam;
      }
    }

    //building smoothing 1D splines for each vector component
    for(int j = 0; j < 3; j++)
      Smspline<Type>((unsigned)points[i]->size(), splinetype, 0, x[j], y[j], rho[j], 0, 0, aa[j], zm[j], 0, sp[j], dsp[j], d2sp[j]);


    //new tesselation parameter's step
    Type step = (1. * points[i]->size() - 1) / (numpnts - 1);
    unsigned count = 0;
    //mew tesselation result
    std::vector<V3d<Type> > newpoints;
    newpoints.resize(numpnts);

    //filling new tesselation values calculating spline values
    for(Type arg = 0; count < numpnts; count++, arg+=step)
    {
      for(int j = 0; j < 3; j++)
      {
        Smspline<Type>((unsigned)points[i]->size(), splinetype, 1, x[j], y[j], rho[j], 0, 0, aa[j], zm[j], arg, sp[j], dsp[j], d2sp[j]);
      }
      newpoints[count] = sp;
    }
    //last point is the equal to first in case of looped spline
    if(splinetype == 3)
      newpoints[numpnts - 1] = newpoints[0];


    //rearranging of new tesselation to avoid screwing of 2D grid
    unsigned refindex = 0;
    if(!skiprearrange)
    {
      Type refdist = std::numeric_limits<Type>::max();
      for(unsigned j = 0; j < numpnts; j++)
      {
        Type dist = newpoints[j] | refpoint;
        if(refdist > dist)
        {
          refdist = dist;
          refindex = j;
        }
      }
    }

    refpoint = newpoints[refindex];


    //filling final result
    for(count = 0; count < numpnts; count++)
    {
      result[count + i * numpnts] = newpoints[(refindex + count) % (numpnts)];
    }
  }
}

//building of splined surface
template <class Type>
void createSurface(const std::vector<V3d<Type> > &gridValue, unsigned numlines, unsigned numpnts, unsigned splinetypex, unsigned splinetypey, unsigned pointsOut, unsigned linesOut, Type rhoParam, Type sgmParam, vtkPoints *newPts, vtkCellArray *newPolys)
{
  {
    //set of working arrays for each 3D vector component
    ForArray<Type> x[3];
    ForArray<Type> y[3];
    ForArray<Type> z[3];
    ForArray<Type> zl[3];
    ForArray<Type> zr[3];
    ForArray<Type> zu[3];
    ForArray<Type> zd[3];
    ForArray<Type> zxy[3];
    ForArray<Type> rho[3];
    ForArray<Type> sgm[3];
    ForArray<Type> zx[3];
    ForArray<Type> zy[3];
    assert(gridValue.size() == numlines * numpnts);

    for(int i = 0; i < 3; i++)
    {
      x[i].assign(numpnts, Type(0));
      y[i].assign(numlines, Type(0));
      z[i].assign(numpnts * numlines, Type(0));
      zl[i].assign(numlines, Type(0));
      zr[i].assign(numlines, Type(0));
      zu[i].assign(numpnts, Type(0));
      zd[i].assign(numpnts, Type(0));
      zxy[i].assign(numpnts * numlines, Type(0));
      rho[i].assign(numpnts, Type(0));
      sgm[i].assign(numlines, Type(0));
      zx[i].assign(numpnts * numlines, Type(0));
      zy[i].assign(numpnts * numlines, Type(0));
    }

    //initialization with input params
    for(int j = 0; j < 3; j++)
    {
      for(unsigned i = 0; i < numpnts; i++)
      {
        x[j][i] = i;
        zu[j][i] = 0.;
        zd[j][i] = 0.;
        rho[j][i] = rhoParam;
      }
      for(unsigned i = 0; i < numlines; i++)
      {
        y[j][i] = i;
        zl[j][i] = 0.;
        zr[j][i] = 0.;
        sgm[j][i] = sgmParam;
      }
      for(unsigned i = 0; i < gridValue.size(); i++)
      {
        z[j][i] = gridValue[i][j];
      }
      for(int i = 0; i < 4; i++)
        zxy[j][i] = 0.;
    }

    //creating smoothing 2D splines for each 3D vector component
    for(int k = 0; k < 3; k++)
    {
      Type        argx = 0;
      Type        argy = 0;
      V3d<Type> resval;
      //Splint2(numpnts, x[k], (unsigned)numlines, y[k], z[k], zl[k], zr[k], zu[k], zd[k], zx[k], zy[k], zxy[k], 
      //        0, 3, 2, argx, argy, resval[k]);
      Smspl2(numpnts, x[k], (unsigned)numlines, y[k], z[k], zl[k], zr[k], zu[k], zd[k], rho[k], sgm[k], zx[k], zy[k], zxy[k], 
        0, splinetypex, splinetypey, argx, argy, resval[k]);
    }

    //preparing output array
    std::vector<std::vector<V3d<Type> > > output;
    output.resize(linesOut);
    for(unsigned i = 0; i < linesOut; i++)
      output[i].assign(pointsOut, V3d<Type>(Type(0), Type(0), Type(0)));


    //producing new tesselation
    Type stepX = 1.0 * (numpnts - 1) / (pointsOut - 1);
    Type stepY = 1.0 * (numlines - 1) / (linesOut - 1);
    for(unsigned j = 0; j < linesOut; j++)
    {
      for(unsigned i = 0; i < pointsOut; i++)
      {
        Type        argx = i * stepX;
        Type        argy   = j * stepY;
        V3d<Type>   resval;
        for(int k = 0; k < 3; k++)
        {
          /*Splint2(numpnts, x[k], (unsigned)numlines, y[k], z[k], zl[k], zr[k], zu[k], zd[k], zx[k], zy[k], zxy[k], 
          1, 3, 2, argx, argy, resval[k]);*/
          Smspl2(numpnts, x[k], (unsigned)numlines, y[k], z[k], zl[k], zr[k], zu[k], zd[k], rho[k], sgm[k], zx[k], zy[k], zxy[k], 1, splinetypex, splinetypey, argx, argy, resval[k]);
        }
        output[j][i] = resval;
      }
    }
    //looping spline if needed
    if(splinetypex == 3)
    {
      for(unsigned i = 0; i < linesOut; i++)
        output[i][output[i].size() - 1] = output[i][0];
    }
    //looping spline if needed
    if(splinetypey == 3)
    {
      for(unsigned i = 0; i < pointsOut; i++)
        output[linesOut - 1][i] = output[0][i];
    }
    //filling output data
    for(unsigned j = 0; j < linesOut; j++)
    {
      for(unsigned i = 0; i < pointsOut; i++)
      {
        newPts->InsertNextPoint(output[j][i].components);
      }
    }
  }

  //filling output triangles
  int triidx = 0;
  for(unsigned i = 0; i < linesOut - 1; i++)
  {
    for(unsigned j = 0; j < pointsOut - 1; j++)
    {
      vtkIdType                pts[3];
      pts[0] = (i) * (pointsOut) + (j);
      pts[1] = (i + 1) * (pointsOut) + (j);
      pts[2] = (i + 1) * (pointsOut) + (j + 1);
      newPolys->InsertNextCell(3, pts);
      pts[0] = (i) * (pointsOut) + (j);
      pts[1] = (i + 1) * (pointsOut) + (j + 1);
      pts[2] = (i) * (pointsOut) + (j + 1);
      newPolys->InsertNextCell(3, pts);
    }
  }
}



#endif