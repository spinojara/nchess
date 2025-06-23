#include <stdio.h>
#include <string.h>

int main(void) {
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	while (1) {
		char buf[BUFSIZ];
		if (!fgets(buf, sizeof(buf), stdin))
			break;

		if (!strcmp(buf, "quit\n")) {
			break;
		}
		else if (!strcmp(buf, "uci\n")) {
			printf("id name nchessengine\n");
			printf("id author Isak Ellmer\n");
			printf("option name AVeryVeryLongOptionNameCheck type check default true\n");
			printf("option name AVeryVeryLongOptionNameSpin type spin default 12831237 min 0 max 12381024890\n");
			printf("option name AVeryVeryLongOptionNameCombo type combo var AVeryVeryLongVariantName\n");
			printf("option name AVeryVeryVeryLongOptionNameButton type button\n");
			printf("option name AVeryVeryLongOptionNameString type string default AVeryVeryLongString\n");
			for (int i = 1; i <= 32; i++) {
				printf("option name option%d type spin default %d\n", i, i);
			}
			printf("uciok\n");
		}
	}

	return 0;
}
