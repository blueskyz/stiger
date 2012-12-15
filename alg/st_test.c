
#include <errno.h>
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

int test_algHuffmanS(char* iFile, char* oFile)
{
	st_timer timerType = ST_TIMER_MILLI_SEC;
	uint32_t uBeginTime = stTimer(timerType);
	st_hfms* phfms = stHfmSNew();
	if (NULL == phfms){
		stErr("create huffman fail.");
		return -1;
	}

	uint32_t uCalloc = 1024 * 1024 * 100;
	char* contentFile = calloc(1, uCalloc);
	FILE* hFile = fopen(iFile, "r+b");
	if (NULL == hFile){
		stErr("err: %s\n", strerror(errno)); 
		return -1;
	}
	int nRead = 0;
	char buf[1024<<6] = { 0 };
	char* pos = contentFile;
	uint32_t uTotalLen = 0;
	while(nRead = fread(buf, 1, sizeof(buf), hFile)){
		if ((uTotalLen + nRead) >= uCalloc){
			stErr("too large file!\n");
			break;
		}
		memcpy(pos, buf, nRead);
		pos += nRead;
		uTotalLen += nRead;
	}
	stLog("total len %u, err %s\n", uTotalLen, strerror(errno));

	char* output = calloc(1, uCalloc);
	uint32_t outLen = uCalloc;
	stHfmSBuild(phfms, (BYTE*)contentFile, uTotalLen, output, &outLen);
	stHfmSFree(phfms);

	stLog("out size %u", outLen);
	FILE* hOutFile = fopen(oFile, "w+b");
	if (NULL == hOutFile){
		stErr("err: %s\n", strerror(errno)); 
		return -1;
	}
	fwrite(output, 1, outLen, hOutFile);
	fclose(hOutFile);
	uint32_t uEndTime = stTimer(timerType);
	stLog("time %u ms", uEndTime - uBeginTime);

	return 0;
}

int test_algHuffmanS_un(char* iFile, char* oFile)
{
	st_timer timerType = ST_TIMER_MILLI_SEC;
	uint32_t uBeginTime = stTimer(timerType);
	st_hfms* phfms = stHfmSNew();
	if (NULL == phfms){
		stErr("create huffman fail.");
		return -1;
	}

	uint32_t uCalloc = 1024 * 1024 * 100;
	BYTE* contentFile = calloc(1, uCalloc);
	FILE* hFile = fopen(iFile, "r+b");
	if (NULL == hFile){
		stErr("err: %s\n", strerror(errno)); 
		return -1;
	}
	int nRead = 0;
	char buf[1024<<6] = { 0 };
	char* pos = contentFile;
	uint32_t uTotalLen = 0;
	while(nRead = fread(buf, 1, sizeof(buf), hFile)){
		if ((uTotalLen + nRead) >= uCalloc){
			stErr("too large file!\n");
			break;
		}
		memcpy(pos, buf, nRead);
		pos += nRead;
		uTotalLen += nRead;
	}
	stLog("total len %u, err %s\n", uTotalLen, strerror(errno));
	stDebug("0=%u, 1=%u, 2=%u", contentFile[0], contentFile[1], contentFile[4]);

	char* output = calloc(1, uCalloc);
	uint32_t outLen = uCalloc;
	stHfmSUncompress(phfms, (BYTE*)contentFile, uTotalLen, output, &outLen);
	stHfmSFree(phfms);

	stLog("out size %u", outLen);
	FILE* hOutFile = fopen(oFile, "w+b");
	if (NULL == hOutFile){
		stErr("err: %s\n", strerror(errno)); 
		return -1;
	}
	fwrite(output, 1, outLen, hOutFile);
	fclose(hOutFile);
	uint32_t uEndTime = stTimer(timerType);
	stLog("time %u ms", uEndTime - uBeginTime);

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc <= 1){
		usage();
		exit(EXIT_FAILURE);
	}

	char* inputFile = NULL;
	char* outputFile = NULL;
	uint32_t uMode = 0; // 0: compress, 1: uncompress
	int opt;
	while ((opt = getopt(argc, argv, "t:a:i:o:m:h")) != -1) {
		switch (opt) {
			case 'a':
				break;
			case 't':
				// nsecs = atoi(optarg);
				break;
			case 'i':
				inputFile = optarg;
				break;
			case 'o':
				outputFile = optarg;
				break;
			case 'm':
				uMode = atoi(optarg);
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
			default: /* '?' */
				usage();
				exit(EXIT_FAILURE);
		}
	}
	if (NULL == inputFile || NULL == outputFile){
		stErr("input file %s, output file %s", inputFile, outputFile);
		return EXIT_FAILURE;
	}
	stLog("input file %s, output file %s, mode %u", 
			inputFile, outputFile, uMode);

	if (0 == uMode){
		test_algHuffmanS(inputFile, outputFile);
	}
	else {
		test_algHuffmanS_un(inputFile, outputFile);
	}

	// pause
	fgetc(stdin);

	return 0;
}

