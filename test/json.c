#include "lib.h"

__USE_LIBASN

int main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	mmatic *mm = mmatic_create();
	xstr *xs = xstr_create("", mm);
	json *js = json_create(mm);

	while (fgets(buf, BUFSIZ, stdin))
		xstr_append(xs, buf);

	ut *parsed = json_parse(js, xstr_string(xs));
	if (ut_ok(parsed))
		printf("%s", ut_char(parsed));
	else
		printf("%s\n", ut_err(parsed));

	return 0;
}
