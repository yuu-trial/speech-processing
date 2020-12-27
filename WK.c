#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Functions prototypes for FFT & IFFT */

void FFT(double *xr, double *xi, double *Xr, double *Xi, int N);
void IFFT(double *Xr, double *Xi, double *xr, double *xi, int N);

/* implementation of FFT */
void FFT(double *xr, double *xi, double *Xr, double *Xi, int N)
{
  int i, j, k, n, n2;
  double theta, wr, wi;

  static double *rbuf, *ibuf;
  static int bufsize = 0;

  /* memory allocation for buffers */
  if (bufsize != N)
  {
    bufsize = N;
    rbuf = (double *)calloc(bufsize, sizeof(double));
    ibuf = (double *)calloc(bufsize, sizeof(double));
  }

  /* bit reverse of xr[] & xi[] --> store to rbuf[] and ibuf[] */
  i = j = 0;
  rbuf[j] = xr[j];
  ibuf[j] = xi[j];
  for (j = 1; j < N - 1; j++)
  {
    for (k = N / 2; k <= i; k /= 2)
      i -= k;
    i += k;
    rbuf[j] = xr[i];
    ibuf[j] = xi[i];
  }
  rbuf[j] = xr[j];
  ibuf[j] = xi[j];

  /* butterfly calculation */
  theta = -2.0 * M_PI;
  for (n = 1; (n2 = n * 2) <= N; n = n2)
  {
    theta *= 0.5;
    for (i = 0; i < n; i++)
    {
      wr = cos(theta * i);
      wi = sin(theta * i);
      for (j = i; j < N; j += n2)
      {
        k = j + n;
        /*
        X[j] = 1*buf[j] + W*buf[k];
        X[k] = 1*buf[j] - W*buf[k];
        Note : X[], buf[], and W are complex numbers.
        Re{ X[n] } = Xr[n], Im{ X[n] } = Xi[n];
        Re{ buf[n] } = rbuf[n], Im{ buf[n] } = ibuf[n];
        Re{ W } = wr, Im{ W } = wi;
            */
        Xr[j] = rbuf[j] + (wr * rbuf[k] - wi * ibuf[k]); /* ??????????, using wr, wi, rbuf, and ibuf */
        Xi[j] = ibuf[j] + (wr * ibuf[k] + wi * rbuf[k]); /* ??????????, using wr, wi, rbuf, and ibuf */
        Xr[k] = rbuf[j] - (wr * rbuf[k] - wi * ibuf[k]); /* ??????????, using wr, wi, rbuf, and ibuf */
        Xi[k] = ibuf[j] - (wr * ibuf[k] + wi * rbuf[k]); /* ??????????, using wr, wi, rbuf, and ibuf */
      }
    }
    for (i = 0; i < N; i++)
    {
      rbuf[i] = Xr[i];
      ibuf[i] = Xi[i];
    }
  }
  return;
}

/* implementation of IFFT */
void IFFT(double *Xr, double *Xi, double *xr, double *xi, int N)
{
  int i, j, k, n, n2;
  double theta, wr, wi;

  static double *rbuf, *ibuf;
  static int bufsize = 0;

  /* memory allocation for buffers */
  if (bufsize != N)
  {
    bufsize = N;
    rbuf = (double *)calloc(sizeof(double), bufsize);
    ibuf = (double *)calloc(sizeof(double), bufsize);
  }

  /* bit reverse of Xr[] & Xi[] --> store to rbuf[] and ibuf[] */
  i = j = 0;
  rbuf[j] = Xr[j] / N;
  ibuf[j] = Xi[j] / N;
  for (j = 1; j < N - 1; j++)
  {
    for (k = N / 2; k <= i; k /= 2)
      i -= k;
    i += k;
    rbuf[j] = Xr[i] / N;
    ibuf[j] = Xi[i] / N;
  }
  rbuf[j] = Xr[j] / N;
  ibuf[j] = Xi[j] / N;

  /* butterfly calculation */
  theta = 2.0 * M_PI; /* not -2.0*M_PI !!! */
  for (n = 1; (n2 = n * 2) <= N; n = n2)
  {
    theta *= 0.5;
    for (i = 0; i < n; i++)
    {
      wr = cos(theta * i);
      wi = sin(theta * i);
      for (j = i; j < N; j += n2)
      {
        k = j + n;
        /*
		x[j] = 1*buf[j] + W*buf[k];
		x[k] = 1*buf[j] - W*buf[k];
		Note : x[], buf[], and W are complex numbers.
		Re{ x[n] } = xr[n], Im{ x[n] } = xi[n];
		Re{ buf[n] } = rbuf[n], Im{ buf[n] } = ibuf[n];
		Re{ W } = wr, Im{ W } = wi;
	      */
        xr[j] = rbuf[j] + (wr * rbuf[k] - wi * ibuf[k]); /* ??????????, using wr, wi, rbuf, and ibuf */
        xi[j] = ibuf[j] + (wr * ibuf[k] + wi * rbuf[k]); /* ??????????, using wr, wi, rbuf, and ibuf */
        xr[k] = rbuf[j] - (wr * rbuf[k] - wi * ibuf[k]); /* ??????????, using wr, wi, rbuf, and ibuf */
        xi[k] = ibuf[j] - (wr * ibuf[k] + wi * rbuf[k]); /* ??????????, using wr, wi, rbuf, and ibuf */
      }
    }

    for (i = 0; i < N; i++)
    {
      rbuf[i] = xr[i];
      ibuf[i] = xi[i];
    }
  }
  return;
}

int main(int argc, char **argv)
{
  /* check the format of input */
  if (argc != 4)
  {
    fprintf(stderr, "Usage: %s DATfile skip[sample] frame_length[sample]\n", argv[0]);
    exit(1);
  }
  FILE *fpDAT;
  int nskip;
  int framelen;
  int i,j;

  /* check the validity of input */
  if ((fpDAT = fopen(argv[1], "r")) == NULL)
    exit(1);
  if ((nskip = atoi(argv[2])) < 0)
    exit(1);
  if ((framelen = atoi(argv[3])) < 0)
    exit(1);

  fprintf(stderr, "# DATfile = %s\n", argv[1]);
  fprintf(stderr, "# %d samples are skipped.\n", nskip);
  fprintf(stderr, "# 1 frame contains %d sampels.\n", framelen);

  /* memory allocation & initilization */
  /* calloc() puts zero-values for assigned memories. */
  short *sdata = (short *)calloc(framelen, sizeof(short));
  double *corr = (double *)calloc(framelen, sizeof(double));
  double *cori = (double *)calloc(framelen, sizeof(double));
  double *specr = (double *)calloc(framelen, sizeof(double));
  double *speci = (double *)calloc(framelen, sizeof(double));
  if (sdata == NULL)
    exit(1);

  fseek(fpDAT, nskip * sizeof(short), SEEK_SET);
  fread(sdata, sizeof(short), framelen, fpDAT);
  fclose(fpDAT);

  /* windowing */
  for (i = 0; i < framelen; i++)
  {
    sdata[i] = (0.54 - 0.46 * cos(2 * M_PI * i / (framelen - 1))) * sdata[i];
    cori[i] = 0.0;
  }

  /* correlation */
  for (i = 0; i < framelen; i++){
    for (j = 0; j < framelen; j++){
      corr[i] += (double)sdata[j] * sdata[(j+i)%framelen]; //循環させる！
    } //実関数かつ偶関数を満たす
    if (i != 0) 
      corr[i] /= corr[0];
  }
  corr[0] = 1.0;

  /* 対称にする */
  // double tmp = corr[framelen/2];
  // for (i = 0; i < framelen/2; i++){
  //   corr[i+framelen/2] = corr[i];
  // }
  // for (i = 1; i < framelen/2; i++){
  //   corr[i] = corr[framelen-i];
  // }
  // corr[0] = tmp;

  /* FFT */
  FFT(corr, cori, specr, speci, framelen);

  /* plot the result */
  for (i = 0; i < framelen; i++)
  {
    // double spec = log10((specr[i] * specr[i] + speci[i] * speci[i]) / framelen);
    printf("%lf %lf\n", (double)i/framelen*16000, log10(specr[i]));
  }

  return 0;
}
