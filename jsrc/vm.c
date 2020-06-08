/* Copyright 1990-2008, Jsoftware Inc.  All rights reserved.               */
/* Licensed use only. Any other use is in violation of copyright.          */
/*                                                                         */
/* Verbs: "Mathematical" Functions (Irrational, Transcendental, etc.)      */

#include "j.h"
#include "ve.h"
// obsolete #define SLEEF C_AVX2
#if SLEEF
#ifdef _WIN32
#include "..\SLEEF\build\include\sleef.h"
#else
#include "../sleef/include/sleef.h"
#endif
#endif


D jtintpow(J jt,D x,I n){D r=1;
 if(0>n){x=1/x; if(n==IMIN){r=x; n=IMAX;} else n=-n;}  // kludge use r=x; n=-1-n;
 while(n){if(1&n)r*=x; x*=x; n>>=1;}
 R r;
}    /* x^n where x is real and n is integral */

D jtpospow(J jt,D x,D y){
 if(0==y)R 1.0;
 if(0==x)R 0<y?0.0:inf;
 if(0<x){
  if(y== inf)R 1<x?inf:1>x?0.0:1.0;
  if(y==-inf)R 1<x?0.0:1>x?inf:1.0;
  R exp(y*log(x));
 }
 if(y==-inf){ASSERT(-1>x,EVDOMAIN); R 0.0;}
 if(y== inf){ASSERT(-1<x,EVDOMAIN); R 0.0;}
 jt->jerr=EWIMAG;
 R 0;
}    /* x^y where x and y are real */

#define POWXB(u,v)  (v?u:1)
#define POWBX(u,v)  (u?1.0:v<0?inf:!v)
#define POWII(u,v)  intpow((D)u,v)
#define POWID(u,v)  pospow((D)u,v)

APFX(powBI, D,B,I, POWBX ,,R EVOK;)
APFX(powBD, D,B,D, POWBX ,,R EVOK;)
APFX(powIB, I,I,B, POWXB ,,R EVOK;)
APFX(powII, D,I,I, POWII ,,HDR1JERR)
APFX(powID, D,I,D, POWID ,,HDR1JERR)
APFX(powDB, D,D,B, POWXB ,,R EVOK;)
APFX(powZZ, Z,Z,Z, zpow  ,,HDR1JERR)



APFX(cirZZ, Z,Z,Z, zcir  ,NAN0;,HDR1JERRNAN)

// Call SLEEF with no checking
#define TRIGUNLIM(sleeffn)  {AVXATOMLOOP( \
 , \
 u=sleeffn(u); \
 , \
 )}


// Call SLEEF after checking symmetric 2-sided limits.  If comp is not true everywhere, signal err, else call sleeffn
#define TRIGSYMM(limit,comp,err,sleeffn)  {AVXATOMLOOP( \
 __m256d thmax; thmax=_mm256_set1_pd(limit); \
 __m256d absmask; absmask=_mm256_castsi256_pd(_mm256_set1_epi64x(0x7fffffffffffffff)); \
 , \
 ASSERTWR(_mm256_movemask_pd(_mm256_cmp_pd(_mm256_and_pd(u,absmask), thmax,comp))==0,err); \
 u=sleeffn(u); \
 , \
 )}

// Call SLEEF after checking limits, but calculate the value to use then
#define TRIGCLAMP(limit,decls,comp,argmod,sleeffn,resultmod)  {AVXATOMLOOP( \
 __m256d thmax; thmax=_mm256_set1_pd(limit); \
 decls \
 __m256d absmask; absmask=_mm256_castsi256_pd(_mm256_set1_epi64x(0x7fffffffffffffff)); \
 , \
 __m256d outofbounds = _mm256_cmp_pd(u, thmax,comp); \
 argmod \
 u=sleeffn(u); \
 resultmod \
 , \
 )}

#if SLEEF
AHDR1(expD,D,D) {  AVXATOMLOOP(
 ,
 u=Sleef_expd4_u10avx2(u);
 ,
 R EVOK;
 )
}
AHDR1(logD,D,D) {  AVXATOMLOOP(
 __m256d zero; zero=_mm256_setzero_pd();
 ,
 ASSERTWR(_mm256_movemask_pd(_mm256_cmp_pd(u, zero,_CMP_LT_OQ))==0,EWIMAG);
 u=Sleef_logd4_u10avx2(u);
 ,
 R EVOK;
 )
}

AHDR2(powDI,D,D,I) {I v;
 if(n-1==0)  DQ(m,               *z++=intpow(*x,*y); x++; y++; )
 else if(n-1<0)DQ(m, D u=*x++; DQC(n, *z++=intpow( u,*y);      y++;))
 else{  // repeated exponent: use parallel instructions
  DQ(m, v=*y++;  // for each exponent
   AVXATOMLOOP(  // build result in u, which is also the input
    __m256d one = _mm256_set1_pd(1.0);
   ,
    __m256d upow;
    UI rempow;  // power left to take
    if(v>=0){  // positive power
     upow=u; u = one;   // init result to 1 before powers
     rempow=v;
    }else{  // negative power, take recip of u and complement the power
     upow = u = _mm256_div_pd(one,u);  // start power at -1
     rempow=~v;  // subtract one from pos pow since we start with recip (avoids IMIN problem)
    }
    while(rempow){if(rempow&1)u=_mm256_mul_pd(u,upow); upow=_mm256_mul_pd(upow,upow); rempow>>=1;}
   ,
   )
  )
 }      
 HDR1JERR
}

AHDR2(powDD,D,D,D) {D v;
 if(n-1==0) DQ(m, *z++=pospow(*x,*y); x++; y++; )
 else if(n-1<0)DQ(m, D u=*x++; DQC(n, *z++=pospow( u,*y); y++;))
 else{  // repeated exponent: use parallel instructions
  DQ(m, v=*y++;  // for each exponent
   if(v==0){DQ(n, *z++=1.0;) x+=n;}
   else if(ABS(v)==inf){DQ(n, D u=*x++; ASSERT(u>=0,EWIMAG); if(u==1.0)*z=1.0; else{D vv = u>1.0?v:-v;*z=v>0?inf:0.0;} ++z;)}
   else{
    AVXATOMLOOP(  // build result in u, which is also the input
      __m256d zero = _mm256_setzero_pd();
      __m256d vv = _mm256_set1_pd(v);  // 4 copies of exponent
     ,
      ASSERTWR(_mm256_movemask_pd(_mm256_cmp_pd(u, zero,_CMP_LT_OQ))==0,EWIMAG);
      u=Sleef_log2d4_u35avx2(u);
      u=_mm256_mul_pd(u,vv);
      u=Sleef_exp2d4_u35avx2(u);
     ,
    )
   )
  }
 }      
 HDR1JERR
}


#else
AMON(expD,   D,D, *z=*x<EMIN?0.0:EMAX<*x?inf:exp(   *x);)
AMON(logD,   D,D, ASSERTWR(0<=*x,EWIMAG); *z=log(   *x);)
APFX(powDI, D,D,I, intpow,,HDR1JERR)
APFX(powDD, D,D,D, pospow,,HDR1JERR)

#endif
AMON(expI,   D,I, *z=*x<EMIN?0.0:EMAX<*x?inf:exp((D)*x);)
AMON(logI,   D,I, ASSERTWR(0<=*x,EWIMAG); *z=log((D)*x);)

static I jtcirx(J jt,I n,I k,D*z,D*x){D p,t;
 NAN0;
 switch(k){
 default: ASSERTWR(0,EWIMAG);
 case  0: DQ(n, t=*x++; ASSERTWR(ABS(t)<=1.0, EWIMAG ); *z++=sqrt(1.0-t*t);); break;
 case  1: ;
#if SLEEF
TRIGSYMM(THMAX,_CMP_GT_OQ,EVLIMIT,Sleef_sind4_u35avx2)
#else
   DQ(n, t=*x++; ASSERTWR(ABS(t)<THMAX,EVLIMIT); *z++=sin(t););
#endif
   break;
 case  2:  ;
#if SLEEF
TRIGSYMM(THMAX,_CMP_GT_OQ,EVLIMIT,Sleef_cosd4_u35avx2)
#else
 DQ(n, t=*x++; ASSERTWR(ABS(t)<THMAX,EVLIMIT); *z++=cos(t););
#endif
 break;
 case  3:  ;
#if SLEEF
 TRIGSYMM(THMAX,_CMP_GT_OQ,EVLIMIT,Sleef_tand4_u35avx2)
#else
 DQ(n, t=*x++; ASSERTWR(ABS(t)<THMAX,EVLIMIT); *z++=tan(t););       
#endif
 break;
 case  4: DQ(n, t=*x++;                                     *z++=t<-1e8?-t:1e8<t?t:sqrt(t*t+1.0););       break;
 case  5: DQ(n, t=*x++;                                     *z++=t<-EMAX2?infm:EMAX2<t?inf:sinh(t););     break;
 case  6: DQ(n, t=*x++;                                     *z++=t<-EMAX2||    EMAX2<t?inf:cosh(t););     break;
 case  7: ;
#if SLEEF
 TRIGUNLIM(Sleef_tanhd4_u35avx2)
 NAN0;  // SLEEF gives the correct answer but may raise a NaN flag
#else
// math library tanh is slooooow  case  7: DQ(n, t=*x++;                                     *z++=t<-TMAX?-1:TMAX<t?1:tanh(t););           break;
 DQ(n, t=*x++;                                     *z++=t<-TMAX?-1:TMAX<t?1:(1.0-exp(-2*t))/(1.0+exp(-2*t)););
#endif
 break;
 case -1: ;
#if SLEEF
  TRIGSYMM(1.0,_CMP_GT_OQ,EWIMAG,Sleef_asind4_u35avx2)
#else
  DQ(n, t=*x++; ASSERTWR( -1.0<=t&&t<=1.0, EWIMAG ); *z++=asin(t););
#if defined(ANDROID) && (defined(__aarch32__)||defined(__arm__)||defined(__aarch64__))
// NaN bug in android asin()  _1 o. _1
NAN0;
#endif
#endif
 break;
 case -2: ;
#if SLEEF
  TRIGSYMM(1.0,_CMP_GT_OQ,EWIMAG,Sleef_acosd4_u35avx2)
#else
  DQ(n, t=*x++; ASSERTWR( -1.0<=t&&t<=1.0, EWIMAG ); *z++=acos(t););
#endif
  break;
 case -3: ;
#if SLEEF
  TRIGUNLIM(Sleef_atand4_u35avx2)
#else
  DQ(n,                                             *z++=atan(*x++););
#endif
  break;
 case -4: DQ(n, t=*x++; ASSERTWR(t<=-1.0||1.0<=t,  EWIMAG ); *z++=t<-1e8||1e8<t?t:t==-1?0:(t+1)*sqrt((t-1)/(t+1));); break;
 case -5: p=log(2.0); 
           DQ(n, t=*x++; *z++=1.0e8<t?p+log(t):-7.8e3>t?-(p+log(-t)):log(t+sqrt(t*t+1.0)););               break;
 case -6: p=log(2.0); 
           DQ(n, t=*x++; ASSERTWR(          1.0<=t, EWIMAG ); *z++=1.0e8<t?p+log(t):log(t+sqrt(t*t-1.0));); break;
 case -7: DQ(n, t=*x++; ASSERTWR( -1.0<=t&&t<=1.0, EWIMAG ); *z++=0.5*log((1.0+t)/(1.0-t)););              break;
 case  9: DQ(n,         *z++=*x++;);           break;    
 case 10: DQ(n, t=*x++; *z++=ABS(t););         break;
 case 11: DQ(n,         *z++=0.0;);            break;
 case 12: DQ(n,         *z++=0<=*x++?0.0:PI;); break;
 }
// obsolete NAN1V;
 ASSERTWR(!NANTEST,EVNAN);
 R EVOK;
}

AHDR2(cirBD,D,B,D){ASSERTWR(n<=1&&1==m,EWIMAG); n^=REPSGN(n); R cirx(n,   (I)*x,z,y);}
AHDR2(cirID,D,I,D){ASSERTWR(n<=1&&1==m,EWIMAG); n^=REPSGN(n); R cirx(n,   *x,z,y);}

AHDR2(cirDD,D,D,D){I k=(I)jround(*x);
 ASSERTWR(k==*x,EVDOMAIN); 
 ASSERTWR(n<=1&&1==m,EWIMAG); // if more than one x value, retry as general case
 n^=REPSGN(n);   // convert complementary n to nonneg
 R cirx(n,k,z,y);
}


F2(jtlogar2){A z;I t;
 RZ(a&&w); 
 RE(t=maxtype(AT(a),AT(w)));
 if(!(t&XNUM)||jt->xmode==XMEXACT){jt->xmode=XMEXACT; R jtatomic2(JTIPAW,logar1(w),logar1(a),ds(CDIV));}  // better to multiply by recip, but not much, & it makes 0 ^. 0 not fail
 z=rank2ex0(cvt(XNUM,a),cvt(XNUM,w),0L,jtxlog2a); 
 if(z)R z;
 if(jt->jerr==EWIMAG||jt->jerr==EWIRR){RESETERR; jt->xmode=XMEXACT; R divideAW(logar1(w),logar1(a));}
 R 0;
}
    
F2(jtroot){A z;I t;
 RZ(a&&w);
 RE(t=maxtype(AT(a),AT(w)));
 A ma=a; if(TYPESNE(t,AT(a)))RZ(ma=cvt(t,a));
 A mw=w; if(TYPESNE(t,AT(w)))RZ(mw=cvt(t,w));
 if(!(t&XNUM))R expn2(mw,recip(ma));  // not inplaceable - could be IMAG
 z=rank2ex0(ma,mw,0L,jtxroota);
 switch(jt->jerr){
  case EWIMAG: RESETERR; R expn2(cvt(CMPX,w),recip(cvt(CMPX,a)));
  case EWRAT: 
  case EWIRR:  RESETERR; R expn2(cvt(FL,  w),recip(cvt(FL,  a)));
  default:     R z;
}}

F1(jtjdot1){R tymes(a0j1,w);}
F2(jtjdot2){R plus(a,tymes(a0j1,w));}
F1(jtrdot1){R expn1(jdot1(w));}
F2(jtrdot2){R tymes(a,rdot1(w));}


F1(jtpolar){RZ(w); A z; R cvt(SPARSE&AT(w)?SFL:FL,df2(z,v2(10L,12L),w,qq(ds(CCIRCLE),v2(1L,0L))));}

F1(jtrect){A e,z;B b;I r,t;P*wp,*zp;Z c;
 RZ(w); 
 t=AT(w); r=AR(w); RESETRANK;   // Run as infinite rank
 ASSERT(!AN(w)||t&NUMERIC,EVDOMAIN);
 if(t&CMPX){GATV(z,FL,2*AN(w),1+r,AS(w)); AS(z)[r]=2; MC(AV(z),AV(w),AN(z)*sizeof(D)); R z;}
 else if(t&SPARSE){
  b=1&&t&SCMPX;
  GASPARSE(z,b?SFL:t,1,1+r,AS(w)); AS(z)[r]=2;
  wp=PAV(w); zp=PAV(z);
  if(b){e=SPA(wp,e); c=*ZAV(e); ASSERT(FFEQ(c.re,c.im),EVSPARSE); SPB(zp,e,scf(c.re));}
  else SPB(zp,e,ca(SPA(wp,e)));
  SPB(zp,a,ca(SPA(wp,a)));
  SPB(zp,i,ca(SPA(wp,i)));
  SPB(zp,x,rect(SPA(wp,x)));
  R z;
 }else R df2(z,w,num(0),qq(ds(CCOMMA),zeroionei(0)));
}
