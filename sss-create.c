#include "sss.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void usage(const char* self);

void usage(const char* self) {
  printf("usage %s <shares> <threshold> <prefix> <secret\n\twhere shares >= threshold, and\n\tthreshold >= 1, and\n\tlen(secret)==64\n", self);
  exit(1);
}

int main(int argc, char **argv) {
  if(argc!=4) usage(argv[0]);

  int n, t;
  n = atoi(argv[1]);
  if(n<1) usage(argv[0]);
  t = atoi(argv[2]);
  if(n<t) usage(argv[0]);
  fprintf(stderr,"[>] creating %d-%d shares\n", n, t);

  sss_Share shares[n];
  uint8_t data[sss_MLEN];
  if(fread(data, sss_MLEN, 1, stdin)!=1) usage(argv[0]);

  /* Split the secret into 5 shares (with a recombination theshold of 4) */
  sss_create_shares(shares, data, n, t);

  char fname[strlen(argv[3])+16/*hex seqno*/+4/*extension*/+1/*terminating 0*/];
  memcpy(fname, argv[3], strlen(argv[3]));

  long i;
  FILE *f;
  for(i=0;i<n;i++) {
    snprintf(fname+strlen(argv[3]), 21, "%016x.sss", (int) i);
    fprintf(stderr,"[>] writing share %ld to %s\n", i, fname);
    f=fopen(fname,"w");
    if(f==NULL) {
      fprintf(stderr,"could not open %s\nabort\n", fname);
      exit(1);
    }
    if(fwrite(shares[i], sizeof(sss_Share), 1, f)!=1) {
      fprintf(stderr,"failed to write share to %s\nabort\n", fname);
      exit(1);
    }
    fclose(f);
  }

  return 0;
}
