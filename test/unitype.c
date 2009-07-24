#include "lib.h"

__USE_LIBASN

int main(int argc, char *argv[])
{
	mmatic *mm = mmatic_create();

	ut *root = ut_new_thash(NULL, mm);

	ut *list123 = uth_add_tlist(root, "list123", NULL);
	utl_add_int(list123, 123);
	utl_add_int(list123, 357);
	utl_add_char(list123, "foobar52");
	utl_add_bool(list123, false);

	printf("%s\n", ut_char(list123));

	ut *hash321 = uth_add_thash(root, "hash321", NULL);
	uth_add_char(hash321, "ala", "ma");
	uth_add_char(hash321, "kota", "i");
	uth_add_char(hash321, "dużego", "psa");

	printf("%s\n", ut_char(hash321));

	printf("%s\n", ut_char(root));

	return 0;
}
