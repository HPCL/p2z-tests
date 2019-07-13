/*
icc propagate-toz-test.C -o propagate-toz-test.exe -fopenmp -O3
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>

#define nevts 100
#define nb    600
#define bsize 16
#define ntrks nb*bsize
#define smear 0.1
#ifdef _OPENARC_
#pragma openarc #define nevts 100
#pragma openarc #define nb    600
#pragma openarc #define bsize 16
#pragma openarc #define ntrks nb*bsize
#pragma openarc #define N bsize
#endif

#ifndef NITER
#define NITER 100
#endif

size_t PosInMtrx(size_t i, size_t j, size_t D) {
  return i*D+j;
}

size_t SymOffsets33(size_t i) {
  const size_t offs[9] = {0, 1, 3, 1, 2, 4, 3, 4, 5};
  return offs[i];
}

size_t SymOffsets66(size_t i) {
  const size_t offs[36] = {0, 1, 3, 6, 10, 15, 1, 2, 4, 7, 11, 16, 3, 4, 5, 8, 12, 17, 6, 7, 8, 9, 13, 18, 10, 11, 12, 13, 14, 19, 15, 16, 17, 18, 19, 20};
  return offs[i];
}

struct ATRK {
  float par[6];
  float cov[21];
  int q;
  int hitidx[22];
};

struct AHIT {
  float pos[3];
  float cov[6];
};

struct MP1I {
  int data[1*bsize];
};

struct MP22I {
  int data[22*bsize];
};

struct MP3F {
  float data[3*bsize];
};

struct MP6F {
  float data[6*bsize];
};

struct MP3x3SF {
  float data[6*bsize];
};

struct MP6x6SF {
  float data[21*bsize];
};

struct MP6x6F {
  float data[36*bsize];
};

struct MPTRK {
  struct MP6F    par;
  struct MP6x6SF cov;
  struct MP1I    q;
  struct MP22I   hitidx;
};

struct MPHIT {
  struct MP3F    pos;
  struct MP3x3SF cov;
};

float randn(float mu, float sigma) {
  float U1, U2, W, mult;
  static float X1, X2;
  static int call = 0;
  if (call == 1) {
    call = !call;
    return (mu + sigma * (float) X2);
  } do {
    U1 = -1 + ((float) rand () / RAND_MAX) * 2;
    U2 = -1 + ((float) rand () / RAND_MAX) * 2;
    W = pow (U1, 2) + pow (U2, 2);
  }
  while (W >= 1 || W == 0); 
  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult; 
  call = !call; 
  return (mu + sigma * (float) X1);
}

struct MPTRK* bTk(struct MPTRK* tracks, size_t ev, size_t ib) {
  return &(tracks[ib + nb*ev]);
}

const struct MPTRK* bTkC(const struct MPTRK* tracks, size_t ev, size_t ib) {
  return &(tracks[ib + nb*ev]);
}

float q(const struct MP1I* bq, size_t it){
  return (*bq).data[it];
}

float par1(const struct MP6F* bpars, size_t it, size_t ipar){
  return (*bpars).data[it + ipar*bsize];
}
float x1    (const struct MP6F* bpars, size_t it){ return par1(bpars, it, 0); }
float _y1    (const struct MP6F* bpars, size_t it){ return par1(bpars, it, 1); }
float z1    (const struct MP6F* bpars, size_t it){ return par1(bpars, it, 2); }
float ipt1  (const struct MP6F* bpars, size_t it){ return par1(bpars, it, 3); }
float phi1  (const struct MP6F* bpars, size_t it){ return par1(bpars, it, 4); }
float theta1(const struct MP6F* bpars, size_t it){ return par1(bpars, it, 5); }
//
float par2(const struct MPTRK* btracks, size_t it, size_t ipar){
  return par1(&(*btracks).par,it,ipar);
}
float x2    (const struct MPTRK* btracks, size_t it){ return par2(btracks, it, 0); }
float y2    (const struct MPTRK* btracks, size_t it){ return par2(btracks, it, 1); }
float z2    (const struct MPTRK* btracks, size_t it){ return par2(btracks, it, 2); }
float ipt2  (const struct MPTRK* btracks, size_t it){ return par2(btracks, it, 3); }
float phi2  (const struct MPTRK* btracks, size_t it){ return par2(btracks, it, 4); }
float theta2(const struct MPTRK* btracks, size_t it){ return par2(btracks, it, 5); }
//
float par3(const struct MPTRK* tracks, size_t ev, size_t tk, size_t ipar){
  size_t ib = tk/bsize;
  const struct MPTRK* btracks = bTkC(tracks, ev, ib);
  size_t it = tk % bsize;
  return par2(btracks, it, ipar);
}
float x3    (const struct MPTRK* tracks, size_t ev, size_t tk){ return par3(tracks, ev, tk, 0); }
float y3    (const struct MPTRK* tracks, size_t ev, size_t tk){ return par3(tracks, ev, tk, 1); }
float z3    (const struct MPTRK* tracks, size_t ev, size_t tk){ return par3(tracks, ev, tk, 2); }
float ipt3  (const struct MPTRK* tracks, size_t ev, size_t tk){ return par3(tracks, ev, tk, 3); }
float phi3  (const struct MPTRK* tracks, size_t ev, size_t tk){ return par3(tracks, ev, tk, 4); }
float theta3(const struct MPTRK* tracks, size_t ev, size_t tk){ return par3(tracks, ev, tk, 5); }
//
void setpar1(struct MP6F* bpars, size_t it, size_t ipar, float val){
  (*bpars).data[it + ipar*bsize] = val;
}
void setx1    (struct MP6F* bpars, size_t it, float val){ setpar1(bpars, it, 0, val); }
void sety1    (struct MP6F* bpars, size_t it, float val){ setpar1(bpars, it, 1, val); }
void setz1    (struct MP6F* bpars, size_t it, float val){ setpar1(bpars, it, 2, val); }
void setipt1  (struct MP6F* bpars, size_t it, float val){ setpar1(bpars, it, 3, val); }
void setphi1  (struct MP6F* bpars, size_t it, float val){ setpar1(bpars, it, 4, val); }
void settheta1(struct MP6F* bpars, size_t it, float val){ setpar1(bpars, it, 5, val); }
//
void setpar2(struct MPTRK* btracks, size_t it, size_t ipar, float val){
  setpar1(&(*btracks).par,it,ipar,val);
}
void setx2    (struct MPTRK* btracks, size_t it, float val){ setpar2(btracks, it, 0, val); }
void sety2    (struct MPTRK* btracks, size_t it, float val){ setpar2(btracks, it, 1, val); }
void setz2    (struct MPTRK* btracks, size_t it, float val){ setpar2(btracks, it, 2, val); }
void setipt2  (struct MPTRK* btracks, size_t it, float val){ setpar2(btracks, it, 3, val); }
void setphi2  (struct MPTRK* btracks, size_t it, float val){ setpar2(btracks, it, 4, val); }
void settheta2(struct MPTRK* btracks, size_t it, float val){ setpar2(btracks, it, 5, val); }

const struct MPHIT* bHit(const struct MPHIT* hits, size_t ev, size_t ib) {
  return &(hits[ib + nb*ev]);
}
//
float pos1(const struct MP3F* hpos, size_t it, size_t ipar){
  return (*hpos).data[it + ipar*bsize];
}
float x_pos1(const struct MP3F* hpos, size_t it)    { return pos1(hpos, it, 0); }
float y_pos1(const struct MP3F* hpos, size_t it)    { return pos1(hpos, it, 1); }
float z_pos1(const struct MP3F* hpos, size_t it)    { return pos1(hpos, it, 2); }
//
float pos2(const struct MPHIT* hits, size_t it, size_t ipar){
  return pos1(&(*hits).pos,it,ipar);
}
float x_pos2(const struct MPHIT* hits, size_t it)    { return pos2(hits, it, 0); }
float y_pos2(const struct MPHIT* hits, size_t it)    { return pos2(hits, it, 1); }
float z_pos2(const struct MPHIT* hits, size_t it)    { return pos2(hits, it, 2); }
//
float pos3(const struct MPHIT* hits, size_t ev, size_t tk, size_t ipar){
  size_t ib = tk/bsize;
  const struct MPHIT* bhits = bHit(hits, ev, ib);
  size_t it = tk % bsize;
  return pos2(bhits,it,ipar);
}
float x_pos3(const struct MPHIT* hits, size_t ev, size_t tk)    { return pos3(hits, ev, tk, 0); }
float y_pos3(const struct MPHIT* hits, size_t ev, size_t tk)    { return pos3(hits, ev, tk, 1); }
float z_pos3(const struct MPHIT* hits, size_t ev, size_t tk)    { return pos3(hits, ev, tk, 2); }

struct MPTRK* prepareTracks(struct ATRK inputtrk) {
  struct MPTRK* result = (struct MPTRK*) malloc(nevts*nb*sizeof(struct MPTRK)); //fixme, align?
  // store in element order for bunches of bsize matrices (a la matriplex)
  for (size_t ie=0;ie<nevts;++ie) {
    for (size_t ib=0;ib<nb;++ib) {
      for (size_t it=0;it<bsize;++it) {
	//par
	for (size_t ip=0;ip<6;++ip) {
	  result[ib + nb*ie].par.data[it + ip*bsize] = (1+smear*randn(0,1))*inputtrk.par[ip];
	}
	//cov
	for (size_t ip=0;ip<21;++ip) {
	  result[ib + nb*ie].cov.data[it + ip*bsize] = (1+smear*randn(0,1))*inputtrk.cov[ip];
	}
	//q
	result[ib + nb*ie].q.data[it] = inputtrk.q-2*ceil(-0.5 + (float)rand() / RAND_MAX);//fixme check
      }
    }
  }
  return result;
}

struct MPHIT* prepareHits(struct AHIT inputhit) {
  struct MPHIT* result = (struct MPHIT*) malloc(nevts*nb*sizeof(struct MPHIT));  //fixme, align?
  // store in element order for bunches of bsize matrices (a la matriplex)
  for (size_t ie=0;ie<nevts;++ie) {
    for (size_t ib=0;ib<nb;++ib) {
      for (size_t it=0;it<bsize;++it) {
  	//pos
  	for (size_t ip=0;ip<3;++ip) {
  	  result[ib + nb*ie].pos.data[it + ip*bsize] = (1+smear*randn(0,1))*inputhit.pos[ip];
  	}
  	//cov
  	for (size_t ip=0;ip<6;++ip) {
  	  result[ib + nb*ie].cov.data[it + ip*bsize] = (1+smear*randn(0,1))*inputhit.cov[ip];
  	}
      }
    }
  }
  return result;
}

#define N bsize
#pragma acc routine seq nohost
void MultHelixPropEndcap(const struct MP6x6F* A, const struct MP6x6SF* B, struct MP6x6F* C) {
  const float* a = (*A).data; //ASSUME_ALIGNED(a, 64);
  const float* b = (*B).data; //ASSUME_ALIGNED(b, 64);
  float* c = (*C).data;       //ASSUME_ALIGNED(c, 64);
#pragma acc loop seq
  for (int n = 0; n < N; ++n)
  {
    c[ 0*N+n] = b[ 0*N+n] + a[ 2*N+n]*b[ 3*N+n] + a[ 3*N+n]*b[ 6*N+n] + a[ 4*N+n]*b[10*N+n] + a[ 5*N+n]*b[15*N+n];
    c[ 1*N+n] = b[ 1*N+n] + a[ 2*N+n]*b[ 4*N+n] + a[ 3*N+n]*b[ 7*N+n] + a[ 4*N+n]*b[11*N+n] + a[ 5*N+n]*b[16*N+n];
    c[ 2*N+n] = b[ 3*N+n] + a[ 2*N+n]*b[ 5*N+n] + a[ 3*N+n]*b[ 8*N+n] + a[ 4*N+n]*b[12*N+n] + a[ 5*N+n]*b[17*N+n];
    c[ 3*N+n] = b[ 6*N+n] + a[ 2*N+n]*b[ 8*N+n] + a[ 3*N+n]*b[ 9*N+n] + a[ 4*N+n]*b[13*N+n] + a[ 5*N+n]*b[18*N+n];
    c[ 4*N+n] = b[10*N+n] + a[ 2*N+n]*b[12*N+n] + a[ 3*N+n]*b[13*N+n] + a[ 4*N+n]*b[14*N+n] + a[ 5*N+n]*b[19*N+n];
    c[ 5*N+n] = b[15*N+n] + a[ 2*N+n]*b[17*N+n] + a[ 3*N+n]*b[18*N+n] + a[ 4*N+n]*b[19*N+n] + a[ 5*N+n]*b[20*N+n];
    c[ 6*N+n] = b[ 1*N+n] + a[ 8*N+n]*b[ 3*N+n] + a[ 9*N+n]*b[ 6*N+n] + a[10*N+n]*b[10*N+n] + a[11*N+n]*b[15*N+n];
    c[ 7*N+n] = b[ 2*N+n] + a[ 8*N+n]*b[ 4*N+n] + a[ 9*N+n]*b[ 7*N+n] + a[10*N+n]*b[11*N+n] + a[11*N+n]*b[16*N+n];
    c[ 8*N+n] = b[ 4*N+n] + a[ 8*N+n]*b[ 5*N+n] + a[ 9*N+n]*b[ 8*N+n] + a[10*N+n]*b[12*N+n] + a[11*N+n]*b[17*N+n];
    c[ 9*N+n] = b[ 7*N+n] + a[ 8*N+n]*b[ 8*N+n] + a[ 9*N+n]*b[ 9*N+n] + a[10*N+n]*b[13*N+n] + a[11*N+n]*b[18*N+n];
    c[10*N+n] = b[11*N+n] + a[ 8*N+n]*b[12*N+n] + a[ 9*N+n]*b[13*N+n] + a[10*N+n]*b[14*N+n] + a[11*N+n]*b[19*N+n];
    c[11*N+n] = b[16*N+n] + a[ 8*N+n]*b[17*N+n] + a[ 9*N+n]*b[18*N+n] + a[10*N+n]*b[19*N+n] + a[11*N+n]*b[20*N+n];
    c[12*N+n] = 0;
    c[13*N+n] = 0;
    c[14*N+n] = 0;
    c[15*N+n] = 0;
    c[16*N+n] = 0;
    c[17*N+n] = 0;
    c[18*N+n] = b[ 6*N+n];
    c[19*N+n] = b[ 7*N+n];
    c[20*N+n] = b[ 8*N+n];
    c[21*N+n] = b[ 9*N+n];
    c[22*N+n] = b[13*N+n];
    c[23*N+n] = b[18*N+n];
    c[24*N+n] = a[26*N+n]*b[ 3*N+n] + a[27*N+n]*b[ 6*N+n] + b[10*N+n] + a[29*N+n]*b[15*N+n];
    c[25*N+n] = a[26*N+n]*b[ 4*N+n] + a[27*N+n]*b[ 7*N+n] + b[11*N+n] + a[29*N+n]*b[16*N+n];
    c[26*N+n] = a[26*N+n]*b[ 5*N+n] + a[27*N+n]*b[ 8*N+n] + b[12*N+n] + a[29*N+n]*b[17*N+n];
    c[27*N+n] = a[26*N+n]*b[ 8*N+n] + a[27*N+n]*b[ 9*N+n] + b[13*N+n] + a[29*N+n]*b[18*N+n];
    c[28*N+n] = a[26*N+n]*b[12*N+n] + a[27*N+n]*b[13*N+n] + b[14*N+n] + a[29*N+n]*b[19*N+n];
    c[29*N+n] = a[26*N+n]*b[17*N+n] + a[27*N+n]*b[18*N+n] + b[19*N+n] + a[29*N+n]*b[20*N+n];
    c[30*N+n] = b[15*N+n];
    c[31*N+n] = b[16*N+n];
    c[32*N+n] = b[17*N+n];
    c[33*N+n] = b[18*N+n];
    c[34*N+n] = b[19*N+n];
    c[35*N+n] = b[20*N+n];
  }
}

#pragma acc routine seq nohost
void MultHelixPropTranspEndcap(const struct MP6x6F* A, const struct MP6x6F* B, struct MP6x6SF* C) {
  const float* a = (*A).data; //ASSUME_ALIGNED(a, 64);
  const float* b = (*B).data; //ASSUME_ALIGNED(b, 64);
  float* c = (*C).data;       //ASSUME_ALIGNED(c, 64);
#pragma acc loop seq
  for (int n = 0; n < N; ++n)
  {
    c[ 0*N+n] = b[ 0*N+n] + b[ 2*N+n]*a[ 2*N+n] + b[ 3*N+n]*a[ 3*N+n] + b[ 4*N+n]*a[ 4*N+n] + b[ 5*N+n]*a[ 5*N+n];
    c[ 1*N+n] = b[ 6*N+n] + b[ 8*N+n]*a[ 2*N+n] + b[ 9*N+n]*a[ 3*N+n] + b[10*N+n]*a[ 4*N+n] + b[11*N+n]*a[ 5*N+n];
    c[ 2*N+n] = b[ 7*N+n] + b[ 8*N+n]*a[ 8*N+n] + b[ 9*N+n]*a[ 9*N+n] + b[10*N+n]*a[10*N+n] + b[11*N+n]*a[11*N+n];
    c[ 3*N+n] = b[12*N+n] + b[14*N+n]*a[ 2*N+n] + b[15*N+n]*a[ 3*N+n] + b[16*N+n]*a[ 4*N+n] + b[17*N+n]*a[ 5*N+n];
    c[ 4*N+n] = b[13*N+n] + b[14*N+n]*a[ 8*N+n] + b[15*N+n]*a[ 9*N+n] + b[16*N+n]*a[10*N+n] + b[17*N+n]*a[11*N+n];
    c[ 5*N+n] = 0;
    c[ 6*N+n] = b[18*N+n] + b[20*N+n]*a[ 2*N+n] + b[21*N+n]*a[ 3*N+n] + b[22*N+n]*a[ 4*N+n] + b[23*N+n]*a[ 5*N+n];
    c[ 7*N+n] = b[19*N+n] + b[20*N+n]*a[ 8*N+n] + b[21*N+n]*a[ 9*N+n] + b[22*N+n]*a[10*N+n] + b[23*N+n]*a[11*N+n];
    c[ 8*N+n] = 0;
    c[ 9*N+n] = b[21*N+n];
    c[10*N+n] = b[24*N+n] + b[26*N+n]*a[ 2*N+n] + b[27*N+n]*a[ 3*N+n] + b[28*N+n]*a[ 4*N+n] + b[29*N+n]*a[ 5*N+n];
    c[11*N+n] = b[25*N+n] + b[26*N+n]*a[ 8*N+n] + b[27*N+n]*a[ 9*N+n] + b[28*N+n]*a[10*N+n] + b[29*N+n]*a[11*N+n];
    c[12*N+n] = 0;
    c[13*N+n] = b[27*N+n];
    c[14*N+n] = b[26*N+n]*a[26*N+n] + b[27*N+n]*a[27*N+n] + b[28*N+n] + b[29*N+n]*a[29*N+n];
    c[15*N+n] = b[30*N+n] + b[32*N+n]*a[ 2*N+n] + b[33*N+n]*a[ 3*N+n] + b[34*N+n]*a[ 4*N+n] + b[35*N+n]*a[ 5*N+n];
    c[16*N+n] = b[31*N+n] + b[32*N+n]*a[ 8*N+n] + b[33*N+n]*a[ 9*N+n] + b[34*N+n]*a[10*N+n] + b[35*N+n]*a[11*N+n];
    c[17*N+n] = 0;
    c[18*N+n] = b[33*N+n];
    c[19*N+n] = b[32*N+n]*a[26*N+n] + b[33*N+n]*a[27*N+n] + b[34*N+n] + b[35*N+n]*a[29*N+n];
    c[20*N+n] = b[35*N+n];
  }
}

#pragma acc routine seq nohost
void propagateToZ(const struct MP6x6SF* inErr, const struct MP6F* inPar,
		  const struct MP1I* inChg, const struct MP3F* msP,
	                struct MP6x6SF* outErr, struct MP6F* outPar) {
  //
  struct MP6x6F errorProp, temp;
#pragma acc loop seq
  for (size_t it=0;it<bsize;++it) {	
    const float zout = z_pos1(msP,it);
    const float k = q(inChg,it)*100/3.8;
    const float deltaZ = zout - z1(inPar,it);
    const float pt = 1./ipt1(inPar,it);
    const float cosP = cosf(phi1(inPar,it));
    const float sinP = sinf(phi1(inPar,it));
    const float cosT = cosf(theta1(inPar,it));
    const float sinT = sinf(theta1(inPar,it));
    const float pxin = cosP*pt;
    const float pyin = sinP*pt;
    const float alpha = deltaZ*sinT*ipt1(inPar,it)/(cosT*k);
    const float sina = sinf(alpha); // this can be approximated;
    const float cosa = cosf(alpha); // this can be approximated;
    setx1(outPar,it, x1(inPar,it) + k*(pxin*sina - pyin*(1.-cosa)) );
    sety1(outPar,it, _y1(inPar,it) + k*(pyin*sina + pxin*(1.-cosa)) );
    setz1(outPar,it,zout);
    setipt1(outPar,it, ipt1(inPar,it));
    setphi1(outPar,it, phi1(inPar,it)+alpha );
    settheta1(outPar,it, theta1(inPar,it) );
    
    const float sCosPsina = sinf(cosP*sina);
    const float cCosPsina = cosf(cosP*sina);
    
    for (size_t i=0;i<6;++i) errorProp.data[bsize*PosInMtrx(i,i,6) + it] = 1.;
    errorProp.data[bsize*PosInMtrx(0,2,6) + it] = cosP*sinT*(sinP*cosa*sCosPsina-cosa)/cosT;
    errorProp.data[bsize*PosInMtrx(0,3,6) + it] = cosP*sinT*deltaZ*cosa*(1.-sinP*sCosPsina)/(cosT*ipt1(inPar,it))-k*(cosP*sina-sinP*(1.-cCosPsina))/(ipt1(inPar,it)*ipt1(inPar,it));
    errorProp.data[bsize*PosInMtrx(0,4,6) + it] = (k/ipt1(inPar,it))*(-sinP*sina+sinP*sinP*sina*sCosPsina-cosP*(1.-cCosPsina));
    errorProp.data[bsize*PosInMtrx(0,5,6) + it] = cosP*deltaZ*cosa*(1.-sinP*sCosPsina)/(cosT*cosT);
    errorProp.data[bsize*PosInMtrx(1,2,6) + it] = cosa*sinT*(cosP*cosP*sCosPsina-sinP)/cosT;
    errorProp.data[bsize*PosInMtrx(1,3,6) + it] = sinT*deltaZ*cosa*(cosP*cosP*sCosPsina+sinP)/(cosT*ipt1(inPar,it))-k*(sinP*sina+cosP*(1.-cCosPsina))/(ipt1(inPar,it)*ipt1(inPar,it));
    errorProp.data[bsize*PosInMtrx(1,4,6) + it] = (k/ipt1(inPar,it))*(-sinP*(1.-cCosPsina)-sinP*cosP*sina*sCosPsina+cosP*sina);
    errorProp.data[bsize*PosInMtrx(1,5,6) + it] = deltaZ*cosa*(cosP*cosP*sCosPsina+sinP)/(cosT*cosT);
    errorProp.data[bsize*PosInMtrx(4,2,6) + it] = -ipt1(inPar,it)*sinT/(cosT*k);
    errorProp.data[bsize*PosInMtrx(4,3,6) + it] = sinT*deltaZ/(cosT*k);
    errorProp.data[bsize*PosInMtrx(4,5,6) + it] = ipt1(inPar,it)*deltaZ/(cosT*cosT*k);
  }
  //
  MultHelixPropEndcap(&errorProp, inErr, &temp);
  MultHelixPropTranspEndcap(&errorProp, &temp, outErr);
}

int main (int argc, char* argv[]) {

   int itr;
   struct ATRK inputtrk = {
     {-12.806846618652344, -7.723824977874756, 38.13014221191406,0.23732035065189902, -2.613372802734375, 0.35594117641448975},
     {6.290299552347278e-07,4.1375109560704004e-08,7.526661534029699e-07,2.0973730840978533e-07,1.5431574240665213e-07,9.626245400795597e-08,-2.804026640189443e-06,
      6.219111130687595e-06,2.649119409845118e-07,0.00253512163402557,-2.419662877381737e-07,4.3124190760040646e-07,3.1068903991780678e-09,0.000923913115050627,
      0.00040678296006807003,-7.755406890332818e-07,1.68539375883925e-06,6.676875566525437e-08,0.0008420574605423793,7.356584799406111e-05,0.0002306247719158348},
     1,
     {1, 0, 17, 16, 36, 35, 33, 34, 59, 58, 70, 85, 101, 102, 116, 117, 132, 133, 152, 169, 187, 202}
   };

   struct AHIT inputhit = {
     {-20.7824649810791, -12.24150276184082, 57.8067626953125},
     {2.545517190810642e-06,-2.6680759219743777e-06,2.8030024168401724e-06,0.00014160551654640585,0.00012282167153898627,11.385087966918945}
   };

   printf("track in pos: %f, %f, %f \n", inputtrk.par[0], inputtrk.par[1], inputtrk.par[2]);
   printf("track in cov: %.2e, %.2e, %.2e \n", inputtrk.cov[SymOffsets66(PosInMtrx(0,0,6))],
	                                       inputtrk.cov[SymOffsets66(PosInMtrx(1,1,6))],
	                                       inputtrk.cov[SymOffsets66(PosInMtrx(2,2,6))]);
   printf("hit in pos: %f %f %f \n", inputhit.pos[0], inputhit.pos[1], inputhit.pos[2]);
   
   printf("produce nevts=%i ntrks=%i smearing by=%f \n", nevts, ntrks, smear);
   printf("NITER=%d\n", NITER);
   
   struct MPTRK* trk = prepareTracks(inputtrk);
   struct MPHIT* hit = prepareHits(inputhit);

   printf("done preparing!\n");
   
   struct MPTRK* outtrk = (struct MPTRK*) malloc(nevts*nb*sizeof(struct MPTRK));

   // for (size_t ie=0;ie<nevts;++ie) {
   //   for (size_t it=0;it<ntrks;++it) {
   //     printf("ie=%lu it=%lu\n",ie,it);
   //     printf("hx=%f\n",x(&hit,ie,it));
   //     printf("hy=%f\n",y(&hit,ie,it));
   //     printf("hz=%f\n",z(&hit,ie,it));
   //     printf("tx=%f\n",x(&trk,ie,it));
   //     printf("ty=%f\n",y(&trk,ie,it));
   //     printf("tz=%f\n",z(&trk,ie,it));
   //   }
   // }
  
   long start, end;
   long start2, end2;
   struct timeval timecheck;

   printf("Size of struct MPTRK trk[] = %ld\n", nevts*nb*sizeof(struct MPTRK));
   printf("Size of struct MPTRK outtrk[] = %ld\n", nevts*nb*sizeof(struct MPTRK));
   printf("Size of struct struct MPHIT hit[] = %ld\n", nevts*nb*sizeof(struct MPHIT));

   gettimeofday(&timecheck, NULL);
   start2 = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

#pragma acc data copyin(trk[0:nevts*nb], hit[0:nevts*nb]) copyout(outtrk[0:nevts*nb])
{
   gettimeofday(&timecheck, NULL);
   start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;
   for(itr=0; itr<NITER; itr++) {
#pragma acc parallel loop gang worker collapse(2) present(trk, hit, outtrk)
   for (size_t ie=0;ie<nevts;++ie) { // loop over events
     for (size_t ib=0;ib<nb;++ib) { // loop over bunches of tracks
       //
       const struct MPTRK* btracks = bTkC(trk, ie, ib);
       const struct MPHIT* bhits = bHit(hit, ie, ib);
       struct MPTRK* obtracks = bTk(outtrk, ie, ib);
       //
       propagateToZ(&(*btracks).cov, &(*btracks).par, &(*btracks).q, &(*bhits).pos, &(*obtracks).cov, &(*obtracks).par); // vectorized function
    }
   }
   } //end of itr loop
   gettimeofday(&timecheck, NULL);
   end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;
}

   gettimeofday(&timecheck, NULL);
   end2 = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

   // for (size_t ie=0;ie<nevts;++ie) {
   //   for (size_t it=0;it<ntrks;++it) {
   //     printf("ie=%lu it=%lu\n",ie,it);
   //     printf("tx=%f\n",x(&outtrk,ie,it));
   //     printf("ty=%f\n",y(&outtrk,ie,it));
   //     printf("tz=%f\n",z(&outtrk,ie,it));
   //   }
   // }
   
   printf("done ntracks=%i tot time=%f (s) time/trk=%e (s)\n", nevts*ntrks, (end-start)*0.001, (end-start)*0.001/(nevts*ntrks));
   printf("data region time=%f (s)\n", (end2-start2)*0.001);
   printf("memory transter time=%f (s)\n", ((end2-start2) - (end-start))*0.001);

   float avgx = 0, avgy = 0, avgz = 0;
   float avgdx = 0, avgdy = 0, avgdz = 0;
   for (size_t ie=0;ie<nevts;++ie) {
     for (size_t it=0;it<ntrks;++it) {
       float x_ = x3(outtrk,ie,it);
       float y_ = y3(outtrk,ie,it);
       float z_ = z3(outtrk,ie,it);
       avgx += x_;
       avgy += y_;
       avgz += z_;
       float hx_ = x_pos3(hit,ie,it);
       float hy_ = y_pos3(hit,ie,it);
       float hz_ = z_pos3(hit,ie,it);
       avgdx += (x_-hx_)/x_;
       avgdy += (y_-hy_)/y_;
       avgdz += (z_-hz_)/z_;
     }
   }
   avgx = avgx/((float)nevts*ntrks);
   avgy = avgy/((float)nevts*ntrks);
   avgz = avgz/((float)nevts*ntrks);
   avgdx = avgdx/((float)nevts*ntrks);
   avgdy = avgdy/((float)nevts*ntrks);
   avgdz = avgdz/((float)nevts*ntrks);

   float stdx = 0, stdy = 0, stdz = 0;
   float stddx = 0, stddy = 0, stddz = 0;
   for (size_t ie=0;ie<nevts;++ie) {
     for (size_t it=0;it<ntrks;++it) {
       float x_ = x3(outtrk,ie,it);
       float y_ = y3(outtrk,ie,it);
       float z_ = z3(outtrk,ie,it);
       stdx += (x_-avgx)*(x_-avgx);
       stdy += (y_-avgy)*(y_-avgy);
       stdz += (z_-avgz)*(z_-avgz);
       float hx_ = x_pos3(hit,ie,it);
       float hy_ = y_pos3(hit,ie,it);
       float hz_ = z_pos3(hit,ie,it);
       stddx += ((x_-hx_)/x_-avgdx)*((x_-hx_)/x_-avgdx);
       stddy += ((y_-hy_)/y_-avgdy)*((y_-hy_)/y_-avgdy);
       stddz += ((z_-hz_)/z_-avgdz)*((z_-hz_)/z_-avgdz);
     }
   }

   stdx = sqrtf(stdx/((float)nevts*ntrks));
   stdy = sqrtf(stdy/((float)nevts*ntrks));
   stdz = sqrtf(stdz/((float)nevts*ntrks));
   stddx = sqrtf(stddx/((float)nevts*ntrks));
   stddy = sqrtf(stddy/((float)nevts*ntrks));
   stddz = sqrtf(stddz/((float)nevts*ntrks));

   printf("track x avg=%f std/avg=%f\n", avgx, fabs(stdx/avgx));
   printf("track y avg=%f std/avg=%f\n", avgy, fabs(stdy/avgy));
   printf("track z avg=%f std/avg=%f\n", avgz, fabs(stdz/avgz));
   printf("track dx/x avg=%f std=%f\n", avgdx, stddx);
   printf("track dy/y avg=%f std=%f\n", avgdy, stddy);
   printf("track dz/z avg=%f std=%f\n", avgdz, stddz);

   free(trk);
   free(hit);
   free(outtrk);

   return 0;
}
