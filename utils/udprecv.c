/*
 * write each received line from UDP to stdout
 *
 * Copyright (C) 2009 ASN Sp. z o.o.
 * Author: Pawel Foremski <pjf@asn.pl>
 *
 * All rights reserved
 */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

#include "lib.h"

__USE_LIBASN

mmatic *mm;

void *udp_sender;
char *bindif = "";
char *ip = "127.0.0.1";
char *port = "10000";

/** Prints usage help screen */
static void help(void)
{
	printf("Usage: udprecv <ip> <port> [OPTIONS]\n");
	printf("\n");
	printf("  Write each line received by UDP to stdout\n");
	printf("\n");
	printf("Options:\n");
	printf("  -i,--bind=<if>   bind to given interface\n");
	printf("  --verbose        be verbose\n");
	printf("  --debug=<num>    set debugging level\n");
	printf("  --help,-h        show this usage help screen\n");
	printf("  --version,-v     show version and copying information\n");
	return;
}

/** Prints version and copying information. */
static void version(void)
{
	printf("udprecv 0.1\n");
	printf("Copyright (C) 2009 ASN Sp. z o.o.\n");
	printf("All rights reserved.\n");
	return;
}

/** Parses Command Line Arguments.
 * @param  argc  argc passed from main()
 * @param  argv  argv passed from main()
 * @retval 0     error, main() should exit (eg. wrong arg. given)
 * @retval 1     ok
 * @retval 2     ok, but main() should exit (eg. on --version or --help)
 * @note   sets  F.fcdir */
static int parse_argv(int argc, char *argv[])
{
	int i, c;

	static char *short_opts = "hvi:";
	static struct option long_opts[] = {
		/* name, has_arg, NULL, short_ch */
		{ "verbose",    0, NULL,  1  },
		{ "debug",      1, NULL,  2  },
		{ "help",       0, NULL,  3  },
		{ "version",    0, NULL,  4  },
		{ "bind",       1, NULL,  5  },
		{ 0, 0, 0, 0 }
	};

	for (;;) {
		c = getopt_long(argc, argv, short_opts, long_opts, &i);
		if (c == -1) break; /* end of options */

		switch (c) {
			case  1 : debug = 5; break;
			case  2 : debug = atoi(optarg); break;
			case 'h':
			case  3 : help(); return 2;
			case 'v':
			case  4 : version(); return 2;
			case 'i':
			case  5 : bindif = optarg; break;
			default: help(); return 0;
		}
	}

	if (argc - optind < 2)
		{ fprintf(stderr, "Not enough arguments\n"); help(); return 0; }

	ip   = argv[optind];
	port = argv[optind + 1];

	return 1;
}

void input(const char *line, void *prv)
{
	fputs(line, stdout);
}

int main(int argc, char *argv[])
{
	switch (parse_argv(argc, argv)) { case 0: exit(1); case 2: exit(0); }

	signal(SIGPIPE, SIG_IGN);
	setvbuf(stdin, 0, _IOLBF, 0);
	setvbuf(stdout, 0, _IOLBF, 0);
	setvbuf(stderr, 0, _IOLBF, 0);

	mm = mmatic_create();

	asn_loop_init();
	asn_loop_listen_udp(bindif, ip, port, input, NULL);
	asn_loop(100);

	return 1;
}

/* for Vim autocompletion:
 * vim: path=.,/usr/include,/usr/local/include,~/local/include
 */
