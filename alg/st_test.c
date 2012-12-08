
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
  char output[102400] = { '\0' };
  //const char content[] = "abababababababababccc,kkkkk9999899111222223434343434343434";
  // const char content[] = "aaa";
  // const char content[] = "aacabbaabcb";
  const char content[] = "We emphasize libraries that work well with the C++ Standard Library. Boost libraries are intended to be widely useful, and usable across a broad spectrum of applications. The Boost license encourages both commercial and non-commercial use, We aim to establish 'existing practice' and provide reference implementations so that Boost libraries are suitable for eventual standardization. Ten Boost libraries are included in the C++ Standards Committee's Library Technical Report (TR1) and in the new C++11 Standard. C++11 also includes several more Boost libraries in addition to those from TR1. More Boost libraries are proposed for TR2 aacabbaabcb Boost works on almost any modern operating system, including UNIX and Windows variants. Follow the Getting Started Guide to download and install Boost. Popular Linux and Unix distributions such as Fedora, Debian, and NetBSD include pre-built Boost packages. Boost may also already be available on your organization's internal web serveri This document is designed to be an extremely gentle introduction, so we included a fair amount of material that may already be very familiar to you. To keep things simple, we also left out some information intermediate and advanced users will probably want. At the end of this document, we'll refer you on to resources that can help you pursue these topics further" \
			 "In a word, Productivity. Use of high-quality libraries like Boost speeds initial development, results in fewer bugs, reduces reinvention-of-the-wheel, and cuts long-term maintenance costs. And since Boost libraries tend to become de facto or de jure standards, many programmers are already familiar with them Ten of the Boost libraries are included in the C++ Standard Library's TR1, and so are slated for later full standardization. More Boost libraries are in the pipeline for TR2. Using Boost libraries gives an organization a head-start in adopting new technologies The Boost libraries tend to be new, fresh, and creative designs. They are not copies, clones, or derivations of proprietary libraries. Boost has a firm policy to respect the IP rights of others. The development of Boost libraries is publicly documented via the mailing lists and version control repository. The source code has been inspected by many, many knowledgeable programmers. Each Boost file has a copyright notice and license information. IP issues have been reviewed by the legal teams from some of the corporations which use Boost, and in some cases these lawyers have been kind enough to give Boost feedback on IP issues. There are no guarantees, but those factors all tend to reduce IP risk" \
			 "Businesses and other organizations often prefer to have code developed, maintained, and improved in the open source community when it does not contain technology specific to their application domain, because it allows them to focus more development resources on their core business Individuals contribute for the technical challenge, to hone their technical skills, for the sense of community, as part of their graduate school programs, as a way around geographic isolation, to enhance their employment opportunities, and as advertisements for their consulting services. There are probably as many reasons as there are individuals. Some of the apparently individual contributions come from employees of support companies with contracts from businesses or other organizations who have an interest in seeing that a library is well-maintained." \
			 "Boost doesn't really have any expenses! All the infrastructure is contributed by supporters, such as the Open Systems Lab at Indiana University, SourceForge, Boost Consulting, MetaCommunications, and the individuals, companies, and other organizations who run the regression tests. Borland, HP, Intel, and Microsoft have contributed compilers. And hundreds, or even thousands, of programmers contribute their time. That's what makes Boost possible." \
			 "Boost.Algorithm is a collection of general purpose algorithms. While Boost contains many libraries of data structures, there is no single library for general purpose algorithms. Even though the algorithms are generally useful, many tend to be thought of as I will be soliciting submissions from other developers, as well as looking through the literature for existing algorithms to include. The Adobe Source Library, for example, contains many useful algorithms that already have documentation and test cases. Knuth's The Art of Computer Programming is chock-full of algorithm descriptions, too.\"too small\" for Boost" \
			 "Boost.Asio may be used to perform both synchronous and asynchronous operations on I/O objects such as sockets. Before using Boost.Asio it may be useful to get a conceptual picture of the various parts of Boost.Asio, your program, and how they work together." \
			 "fjaksdjfasf" \
			 "This is the MySQL? Reference Manual. It documents MySQL 5.5 through 5.5.30, as well as MySQL Cluster releases based on version 7.2 of NDBCLUSTER through 5.5.28-ndb-7.2.10. MySQL 5.5 features.  This manual describes features that are not included in every edition of MySQL 5.5; such features may not be included in the edition of MySQL 5.5 licensed to you. If you have any questions about the features included in your edition of MySQL 5.5, refer to your MySQL 5.5 license agreement or contact your Oracle sales representative. " \
			 "– Follow the Ticker on Twitter: @PoliticalTicker Coburn, who served on the Simpson-Bowles fiscal commission and participated in the Gang of Six deficit talks, was one of the first Republicans a couple of years to embrace raising revenue to reduce the deficit. At that time, he wanted to do it through reforming the tax code by eliminating loopholes and deductions that he argued favored the rich and powerful. But now he appears to be the first GOP senator to say publicly he would back increasing the tax rates on the wealthy, as long as that increase is coupled with spending cuts and entitlement reforms." \
			 "Washington (CNN) - In a significant development in the fiscal cliff standoff, Republican Sen. Tom Coburn, a leading deficit hawk, said Wednesday he would support higher tax rates on wealthier Americans as part of a broader deal with President Barack Obama and congressional Democrats to avoid the crisis." \
			 "Coburn thinks its better to raise rates now, which will generate the money needed to get beyond the fiscal cliff and then negotiate broad reforms to the tax code – such as eliminating deductions and loopholes – that both Republicans and Democrats argue is necessary.";

  printf("size=%u, content=%s\n", strlen(content), content);
  st_hfms* phfms = stHfmSNew();
  uint32_t outLen = sizeof(output);
  if (NULL == phfms){
	stErr("create huffman fail.");
	return -1;
  }

  stHfmSBuild(phfms, (BYTE*)content, strlen(content), output, &outLen);
  stHfmSFree(phfms);

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

  // pause
  fgetc(stdin);

  return 0;
}

