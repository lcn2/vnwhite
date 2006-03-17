/*
 * vmwhite - Von Neumann whitener
 *
 * Given pairs of bits on input, produce 0 or 1 bits of output.
 *
 *	0 0 ==> (output nothing)
 *	1 0 ==> output 0 bit
 *	0 1 ==> output 1 bit
 *	1 1 ==> (output nothing)
 *
 * @(#) $Revision$
 * @(#) $Id$
 * @(#) $Source$
 *
 * Copyright (c) 2004 by Landon Curt Noll.  All Rights Reserved.
 *
 * See:
 *	http://en.wikipedia.org/wiki/Hardware_random_number_generator#Software_whitening
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *       supporting documentation
 *       source copies
 *       source works derived from this source
 *       binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo (Landon Curt Noll, http://www.isthe.com/chongo/index.html) /\oo/\
 *
 * Share and enjoy! :-)
 */


#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>

#define OCTET_BITS (8)			/* there are 8 bits in an octet */
#define OCTET_VALS (1<<OCTET_BITS)	/* an octet can have 1 of 2^8 values */
#undef BUILD_TBL	/* define BUILD_TBL to rebuild vn_amt[] & vn_out[] */

/*
 * my vars
 */
static int debug_level = 0;
static char *program = NULL;
static char *usage = "usage: %s [-v level]\n";

/*
 * given octet value i, we output vn_amt[i] bits
 *
 * NOTE: 0 <= vn_amt[i] <= 4
 */
static int vn_amt[OCTET_VALS] = {
 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0,
 1, 2, 2, 1, 2, 3, 3, 2, 2, 3, 3, 2, 1, 2, 2, 1,
 1, 2, 2, 1, 2, 3, 3, 2, 2, 3, 3, 2, 1, 2, 2, 1,
 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0,
 1, 2, 2, 1, 2, 3, 3, 2, 2, 3, 3, 2, 1, 2, 2, 1,
 2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 2, 3, 3, 2,
 2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 2, 3, 3, 2,
 1, 2, 2, 1, 2, 3, 3, 2, 2, 3, 3, 2, 1, 2, 2, 1,
 1, 2, 2, 1, 2, 3, 3, 2, 2, 3, 3, 2, 1, 2, 2, 1,
 2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 2, 3, 3, 2,
 2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 2, 3, 3, 2,
 1, 2, 2, 1, 2, 3, 3, 2, 2, 3, 3, 2, 1, 2, 2, 1,
 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0,
 1, 2, 2, 1, 2, 3, 3, 2, 2, 3, 3, 2, 1, 2, 2, 1,
 1, 2, 2, 1, 2, 3, 3, 2, 2, 3, 3, 2, 1, 2, 2, 1,
 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0,
};

/*
 * given octet value i, we will output the low
 * vn_amt[i] bits of vn_out[i]
 *
 * NOTE: 0 <= vn_out[i] <= 15
 */
static int vn_out[OCTET_VALS] = {
 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
 0x01, 0x03, 0x02, 0x01, 0x03, 0x07, 0x06, 0x03,
 0x02, 0x05, 0x04, 0x02, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
 0x01, 0x03, 0x02, 0x01, 0x03, 0x07, 0x06, 0x03,
 0x02, 0x05, 0x04, 0x02, 0x01, 0x03, 0x02, 0x01,
 0x03, 0x07, 0x06, 0x03, 0x07, 0x0f, 0x0e, 0x07,
 0x06, 0x0d, 0x0c, 0x06, 0x03, 0x07, 0x06, 0x03,
 0x02, 0x05, 0x04, 0x02, 0x05, 0x0b, 0x0a, 0x05,
 0x04, 0x09, 0x08, 0x04, 0x02, 0x05, 0x04, 0x02,
 0x01, 0x03, 0x02, 0x01, 0x03, 0x07, 0x06, 0x03,
 0x02, 0x05, 0x04, 0x02, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
 0x01, 0x03, 0x02, 0x01, 0x03, 0x07, 0x06, 0x03,
 0x02, 0x05, 0x04, 0x02, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
 0x01, 0x03, 0x02, 0x01, 0x03, 0x07, 0x06, 0x03,
 0x02, 0x05, 0x04, 0x02, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
 0x00, 0x01, 0x00, 0x00, 0x01, 0x03, 0x02, 0x01,
 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
};

/*
 * forward function declarations
 */
static void dbg(int level, char *fmt, ...);
#if defined(BUILD_TBL)
static void load_tbl(void);
#endif


/*
 * main
 */
int
main(int argc, char *argv[])
{
    extern char *optarg;	/* option argument */
    extern int optind;		/* argv index of the next arg */
    int c;			/* single octet input buffer */
    unsigned short out = 0;	/* 2 octet output buffer */
    int out_bit_len = 0;	/* number of bits in output buffer */
    int input_octets = 0;	/* octets read on stdin */
    int output_octets = 0;	/* octets written to stdout */
    int i;

    /*
     * parse args
     */
    program = argv[0];
    while ((i = getopt(argc, argv, "v:")) != -1) {
	switch (i) {
	case 'v':
	    debug_level = atoi(optarg);
	    break;
	default:
	    fprintf(stderr, usage, program);
	    exit(1);
	}
    }
    if (optind < argc) {
	fprintf(stderr, usage, program);
	exit(2);
    }

#if defined(BUILD_TBL)
    /*
     * load tables
     */
    load_tbl();
#endif

    /*
     * process stdin according to the Von Neumann whitener algorithm
     */
    while ((c = getchar()) != EOF) {

	/*
	 * Von Neumann whiten the input octet
	 */
	dbg(2, "input octet: 0x%02x", c);
	++input_octets;
	dbg(2, "converted input to %d low order bits of 0x%02x",
	    vn_amt[c], vn_out[c]);
	out |= (vn_out[c] << out_bit_len);
	out_bit_len += vn_amt[c];

	/*
	 * if we have at least a full octet in the output buffer, then write it
	 */
        if (out_bit_len >= OCTET_BITS) {
	    dbg(2, "will output octet: 0x%02x", out & (OCTET_VALS-1));
	    if (putchar(out & (OCTET_VALS-1)) == EOF) {
		dbg(1, "EOF on output");
		break;
	    }
	    ++output_octets;
	    out_bit_len -= OCTET_BITS;
	    out >>= OCTET_BITS;
	}
    }
    dbg(1, "EOF on input");

    /*
     * if remaining bits in output buffer, write it with 0 padding
     */
    dbg(2, "there are %d bits in the output buffer", out_bit_len);
    if (out_bit_len > 0) {
	dbg(2, "final output octet: 0x%02x", out & (OCTET_VALS-1));
	if (putchar(out & (OCTET_VALS-1)) != EOF) {
	    ++output_octets;
        }
    }

    /*
     * optional accounting
     */
    dbg(1, "input octets: %d", input_octets);
    dbg(1, "input bits: %d", input_octets*OCTET_BITS);
    dbg(1, "output octets: %d", output_octets);
    if (out_bit_len == 0) {
	dbg(1, "output bits: %d", output_octets*OCTET_BITS);
	dbg(1, "low order output bits in last octet: %d", OCTET_BITS);
    } else {
	dbg(1, "output bits: %d", (output_octets-1)*OCTET_BITS + out_bit_len);
	dbg(1, "low order output bits in last octet: %d", out_bit_len);
    }

    /*
     * All Done!!! -- Jessica Noll, Age 2
     */
    exit(0);
}


#if defined(BUILD_TBL)
/*
 * load_tbl - load tables
 *
 * NOTE: For optimal performance, use the -v 3 (or higher) debug output to
 *	 compile in a the table as static values.
 */
static void
load_tbl(void)
{
    int amt;		/* number of bits to output */
    unsigned int out;	/* output bits */
    unsigned int pair;	/* bit bits to consider */
    int b;		/* bit number */
    unsigned int i;

    /*
     * load the vn_amt[] and vn_out[] for each possible octet value
     */
    for (i=0; i < OCTET_VALS; ++i) {

	/*
	 * look at the value, 2 bits at a time
	 */
	amt = 0;
	out = 0;
	for (b=0; b < OCTET_BITS; b += 2) {

	    /* get the bit pair */
	    pair = ((i>>b) & 0x03);

	    /* process the bit pair */
	    switch (pair) {
	    case 0:	/* same bits, nothing to output */
		break;
	    case 1:	/* 0 1 ==> output 1 bit */
		out |= (1 << amt);
	        ++amt;
		break;
	    case 2:	/* 1 0 ==> output 0 bit */
		/* out has a 0 bit already, just increase the amount */
	        ++amt;
		break;
	    case 3:	/* same bits, nothing to output */
		break;
	    }
	}

	/*
	 * load table values
	 */
        vn_amt[i] = amt;
        vn_out[i] = out;
    }

    /*
     * output table
     */
    if (debug_level > 3) {
	int j;

        fprintf(stderr, "/*\n");
        fprintf(stderr, " * given octet value i, we output vn_amt[i] bits\n");
	fprintf(stderr, " *\n");
	fprintf(stderr, " * NOTE: 0 <= vn_amt[i] <= 4\n");
        fprintf(stderr, " */\n");
        fprintf(stderr, "static int vn_amt[OCTET_VALS] = {\n");
	for (i=0; i < OCTET_VALS; i += 16) {
	    for (j=0; j < 16; ++j) {
	        fprintf(stderr, " %d,", vn_amt[i+j]);
	    }
	    fputc('\n', stderr);
	}
        fprintf(stderr, "};\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "/*\n");
        fprintf(stderr, " * given octet value i, we will output the low\n");
	fprintf(stderr, " * vn_amt[i] bits of vn_out[i]\n");
	fprintf(stderr, " *\n");
	fprintf(stderr, " * NOTE: 0 <= vn_out[i] <= 15\n");
        fprintf(stderr, " */\n");
        fprintf(stderr, "static int vn_out[OCTET_VALS] = {\n");
	for (i=0; i < OCTET_VALS; i += 8) {
	    for (j=0; j < 8; ++j) {
	        fprintf(stderr, " 0x%02x,", vn_out[i+j]);
	    }
	    fputc('\n', stderr);
	}
        fprintf(stderr, "};\n");
    }
    return;
}
#endif


/*
 * dbg - debug
 */
static void
dbg(int level, char *fmt, ...)
{
    va_list ap;		/* argument list */

    /* do nothing if the debug level is too low */
    if (level < debug_level) {
	return;
    }

    /* firewall */
    if (fmt == NULL) {
	fmt = "((NULL format given))";
    }

    /* variable argument list setup */
    va_start(ap, fmt);

    /* output debug message */
    fprintf(stderr, "Debug[%d]: ", level);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);

    /* cleanup */
    va_end(ap);
    return;
}
