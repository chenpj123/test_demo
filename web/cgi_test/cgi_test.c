#include <stdio.h>

void main()
{
	printf("Content-type: text/html\n\n");
	printf("<html>\n");
	printf("<h1><title>CGI Output</title></h1>\n");
	printf("<body>\n");
	printf("Hello World\n");
	printf("</body>\n");
	printf("</html>\n");
}