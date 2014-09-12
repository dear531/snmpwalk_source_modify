#include <stdio.h>
#include <string.h>
#include <errno.h>
int main(void)
{
	char *commond = "snmpwalk -v 3 -l authNoPriv -u zhangliuying -a MD5 -A zhangliuying 192.168.12.76 .1.3.6.1.4.1.2021.4.11.0 2>&1\necho \"result:$?\"";
	FILE * fp;
	fp = popen(commond, "r");
	if (NULL == fp) {
		fprintf(stdout, "popen error :%s\n", strerror(errno));
		goto finish;
	}
	char buf[1024];
	int i = 0;

	while (NULL != fgets(buf, sizeof(buf), fp)) {
		if (0 == strncasecmp(buf, "USAGE:", strlen("USAGE:"))
				|| 0 == strncasecmp(buf, "result", strlen("result"))) {
			break;
		}
		fprintf(stdout, "%s", buf);
		fprintf(stdout, "i:%d\n", i++);
	}

	if (NULL != fp)
		pclose(fp);
finish:
	return 0;
}
