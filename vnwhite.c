/*
 * vmwhite - Von Neumann whitener
 *
 * For each pair of input bits, produce 0 or 1 bits of output according to:
 *
 *	0 0 ==> (output nothing)
 *	1 0 ==> output 0 bit
 *	0 1 ==> output 1 bit
 *	1 1 ==> (output nothing)
 *
 * From:
 *
 *	http://en.wikipedia.org/wiki/Hardware_random_number_generator#Software_whitening
 *
 * John von Neumann invented a simple algorithm to fix simple bias, and
 * reduce correlation: it considers bits two at a time, taking one of
 * three actions: when two successive bits are the same, they are not used
 * as a random bit, a sequence of 0,1 becomes a 1, and a sequence of 1,0
 * becomes a 0. This eliminates simple bias, and is easy to implement as
 * a computer program or in digital logic. This technique works no matter
 * how the bits have been generated. It cannot assure randomness in its
 * output, however. What it can do is (with significant loss) transform a
 * random stream with a frequency of 1's different from 50% into a stream
 * with that frequency, which is useful with some physical sources. When
 * the random stream has a 50% frequency of 1's to begin with, it reduces
 * the bit rate available by a factor of four, on average.
 *
 * Copyright (c) 2004-2005,2025 by Landon Curt Noll.  All Rights Reserved.
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
 * chongo (Landon Curt Noll) /\oo/\
 *
 * http://www.isthe.com/chongo/index.html
 * https://github.com/lcn2
 *
 * Share and enjoy!  :-)
 */


#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>

#define OCTET_BITS (8)			/* there are 8 bits in an octet */
#define OCTET_VALS (1<<OCTET_BITS)	/* an octet can have 1 of 2^8 values */

/* define BUILD_TBL and run with -v 3 to rebuild vn_amt[] and vn_out[] arrays */
#undef BUILD_TBL
#if defined(BUILD_TBL)
# define CONST
#else
# define CONST const
#endif

/*
 * static vars
 */
static int debug_level = 0;
static char *program = NULL;
static char *usage = "usage: %s [-h] [-v level] [-V]\n";
static char *version = "1.0.0 2025-03-23";

/*
 * given octet value i, we output vn_amt[i] bits
 *
 * NOTE: 0 <= vn_amt[i] <= 4
 */
static CONST int vn_amt[OCTET_VALS] = {
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
static CONST int vn_out[OCTET_VALS] = {
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
 * main - our program
 *
 * NOTE: see usage message and lead comment above
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
    while ((i = getopt(argc, argv, "hv:V")) != -1) {
	switch (i) {
	case 'h':
	    fprintf(stderr, usage, program);
	    exit(2);
	case 'v':
	    debug_level = atoi(optarg);
	    dbg(1, "debug level set to %d", debug_level);
	    break;
	case 'V':
	    fprintf(stderr, "%s\n", version);
	    exit(2);
	    break;
	default:
	    fprintf(stderr, usage, program);
	    exit(3);
	}
    }
    if (optind < argc) {
	fprintf(stderr, usage, program);
	exit(3);
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
    clearerr(stdin);
    clearerr(stdout);
    while ((c = getchar()) != EOF) {

	/*
	 * input accounting
	 */
	dbg(2, "input octet: 0x%02x", c);
	++input_octets;
	dbg(2, "converted input to %d low order bits of 0x%02x",
	    vn_amt[c], vn_out[c]);

	/*
	 * Von Neumann whiten the input octet
	 */
	/* next 2 lines are the core of the Von Neumann whitener algorithm */
	out |= (vn_out[c] << out_bit_len);
	out_bit_len += vn_amt[c];

	/*
	 * if we have a full octet in the output buffer, then write it
	 */
        if (out_bit_len >= OCTET_BITS) {

	    /*
	     * output accounting
	     */
	    dbg(2, "will output octet: 0x%02x", out & (OCTET_VALS-1));
	    if (putchar(out & (OCTET_VALS-1)) == EOF) {
		dbg(1, "end of processing output");
		dbg(1, "%s on output", (feof(stdout) ? "EOF" : "error"));
		break;
	    }
	    ++output_octets;

	    /*
	     * remove the octet that we just wrote from the output buffer
	     */
	    out >>= OCTET_BITS;
	    out_bit_len -= OCTET_BITS;
	}
    }

    /*
     * final accounting
     *
     * NOTE: We could have written any bits remaining in the output buffer.
     *	     Because we must write in whole octets, the result would have
     *	     to be 0-bit padded resulting in an unbalanced output.  In a
     *	     daemon or kernel driver one could just keep around these
     *	     partial bits for next time.  However in the case of a filter
     *	     in a pipe, we must end sometime.  So rather than output
     *	     with non-balanced 0-bit padding, we choose to toss the
     *	     final fractional octet.
     */
    dbg(1, "end of processing input");
    if (c == EOF) {
	dbg(1, "%s on input", (feof(stdin) ? "EOF" : "error"));
    }
    dbg(1, "input octet(s): %d", input_octets);
    dbg(1, "input bit(s): %d", input_octets*OCTET_BITS);
    dbg(1, "output octet(s): %d", output_octets);
    dbg(1, "output bit(s): %d", output_octets*OCTET_BITS);
    dbg(1, "left %d bit(s) behind in the output buffer", out_bit_len);
    if (out_bit_len > 0) {
	dbg(1, "tossing the low order %d output bit(s) of: 0x%02x",
		out_bit_len, out & (OCTET_VALS-1));
    }
    dbg(1, "input bit(s) to output bit(s) ratio: %f",
	    ((output_octets*OCTET_BITS+out_bit_len != 0) ?
		 ((double)(input_octets*OCTET_BITS) /
		   (double)(output_octets*OCTET_BITS+out_bit_len)) :
		 (double)0.0));

    /*
     * All Done!!! -- Jessica Noll, Age 2
     */
    exit(0);
}


#if defined(BUILD_TBL)
/*
 * load_tbl - load the vn_amt[] and vn_out[] tables
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
        fprintf(stderr, "static CONST int vn_amt[OCTET_VALS] = {\n");
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
        fprintf(stderr, "static CONST int vn_out[OCTET_VALS] = {\n");
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
 * dbg - debug output to stderr if -v level is set high enough
 *
 * given:
 *	level	the minimum debug level needed to output the message
 *	fmt	the debug format
 *	...	optional args of the debug format
 */
static void
dbg(int level, char *fmt, ...)
{
    va_list ap;		/* argument list */

    /* do nothing if the debug level is too low */
    if (level > debug_level) {
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
