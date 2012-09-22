
#include <getopt.h>

#include <alg/st_huffman_s.h>

void usage()
{
  const char* usage="usage:  ./st_test <-t mode> <test argu> [-i input file name]\n"
	"  -t mode: algorithm test. [ alg ]\n"
	"  -a algName: algorithm name. [ huffman_s ]\n"
	"  -h: help\n";
  printf("%s", usage);
}

int test_algHuffmanS()
{
  char output[1024] = { '\0' };
  const char* content = "abababababababababccc,kkkkk";
  printf("content=%s\n", content);
  st_hfms* phfms = stHfmSNew();
  uint32_t uLen = 1024;
  if (NULL == phfms){
	stErr("create huffman fail.");
	return -1;
  }

  stHfmSBuild(phfms, (BYTE*)content, strlen(content));
  stHfmSOutput(phfms, (BYTE*)output, &uLen);
  stHfmSFree(phfms);

  // pause
  fgetc(stdin);

  return 0;
}

int main(int argc, char* argv[])
{
  if (argc <= 1){
	usage();
	exit(EXIT_FAILURE);
  }

  int opt;
  while ((opt = getopt(argc, argv, "t:a:h")) != -1) {
	switch (opt) {
	  case 'a':
		break;
	  case 't':
		// nsecs = atoi(optarg);
		break;
	  case 'h':
		usage();
		exit(EXIT_SUCCESS);
	  default: /* '?' */
		usage();
		exit(EXIT_FAILURE);
	}
  }

  test_algHuffmanS();

  return 0;
}

