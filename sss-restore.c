#include "sss.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void usage(const char* self);

void usage(const char* self) {
  printf("usage %s <no-of-shares> <shares >secret\n", self);
  exit(1);
}

int main(int argc, char **argv) {
  if(argc!=2) usage(argv[0]);

  int n;
  n = atoi(argv[1]);
  if(n<1) usage(argv[0]);
  fprintf(stderr,"[>] restoring from %d shares\n", n);

  sss_Share shares[n];
  uint8_t data[sss_MLEN];
  if(fread(shares, sizeof(sss_Share), n, stdin)!=(size_t) n) usage(argv[0]);

  /* /\* Combine some of the shares to restore the original secret *\/ */
  if(sss_combine_shares(data, shares, n)!=0) {
    fprintf(stderr,"failed to restore from shares\n");
    exit(1);
  }

  fwrite(data,sss_MLEN,1,stdout);

  return 0;
}
