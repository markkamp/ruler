//static char *mcard = "1a [bkg]do[efe]fs[  ]gshnu[j ]ln[b ][w ]a[rv]p[ t]vxwayb[ cb]23bac deghe[ f]jkr[ ln]cmcoi stxuwyf4";
//static char *mcardErr = "  c               em        w     u   j        z         e        q      z      o        p        ";
//01a,01b
//static char *ucard = "1abkgdofsgshnjlnbjarvzvxwaybcb23bacdeghefjkrlncmcoipstuwyf4";

//TODO: return types should all be of an error type

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "generic.h"
#include "stage1.h"

static char *mcard = NULL;
static size_t mcardLen = 0;

static char *mcardErr = NULL;
static size_t mcardErrLen = 0;

static char *ucard = NULL;
static size_t ucardLen = 0;

static char mcardStripped[MAX_STR_LENGTH];

static char resultmcard[MAX_STR_LENGTH];
static char resultucard[MAX_STR_LENGTH];

#ifdef MINGW
static size_t strnlen (const char *string, size_t maxlen);
#endif
static int ParseArguments(int argc, char *argv[]);

#ifdef MINGW
/* Required for MINGW. */
static size_t strnlen (const char *string, size_t maxlen)
{
    const char *end = memchr (string, '\0', maxlen);

    return end ? (size_t) (end - string) : maxlen;
}
#endif

static int ParseArguments(int argc, char *argv[])
{
    int retVal = 0;
    size_t strLen = 0;
    uint32_t i = 0;

    if (argc != 4) {
        return -1;
    }

    /* Skip argument 0. */

    /* Parse argument 1: mcard. */
    strLen = strnlen(argv[1], MAX_STR_LENGTH);
    if ((strLen == 0) || (strLen >= MAX_STR_LENGTH)) {
        return -1;
    }

    for (i = 0; i < strLen; i++) {
        if ((IsIgnoreChar(argv[1][i]) == false) &&
            (IsMatchChar(argv[1][i]) == false)) {
            mcard = NULL;
            return -1;
        }
    }
    mcard = argv[1];
    mcardLen = strLen;

    /* Parse argument 2: mcardErr. */
    strLen = strnlen(argv[2], MAX_STR_LENGTH);
    if ((strLen == 0) || (strLen >= MAX_STR_LENGTH)) {
        return -1;
    }

    for (i = 0; i < strLen; i++) {
        if ((argv[2][i] != ' ') &&
            (IsMatchChar(argv[2][i]) == false)) {
            mcardErr = NULL;
            return -1;
        }
    }
    mcardErr = argv[2];
    mcardErrLen = strLen;

    /* mcardLen must be equal to mcardErrLen. */
    if (mcardLen != mcardErrLen) {
        return -1;
    }

    /* Parse argument 3: ucard. */
    strLen = strnlen(argv[3], MAX_STR_LENGTH);
    if ((strLen == 0) || (strLen >= MAX_STR_LENGTH)) {
        return -1;
    }

    for (i = 0; i < strLen; i++) {
        if (IsMatchChar(argv[3][i]) == false) {
            ucard = NULL;
            return -1;
        }
    }
    ucard = argv[3];
    ucardLen = strLen;

    return retVal;
}

int main(int argc, char *argv[])
{
    if (ParseArguments(argc, argv) == -1) {
        return 1;
    }

    printf("\nInput:\n");
    printf("         mcard: %s\n", mcard);
    printf("      mcarderr: %s\n", mcardErr);
    printf("         ucard: %s\n", ucard);

    /* Perform stage 1: the removal of matched string from max length down to length 3. */
    CompareAndMatchStrings(mcard, mcardLen,
                           ucard, ucardLen,
                           resultmcard, sizeof(resultucard),
                           resultucard, sizeof(resultucard),
                           3);

    /* Stage 2. */
    //TODO

    /* Print results. */
    printf("\nResults:\n");

    (void)StripIgnoreChars(mcard, mcardLen, mcardStripped, sizeof(mcardStripped));
    printf("     str mcard: %s\n", mcardStripped);

    printf("         ucard: %s\n", ucard);
    printf("   resultucard: %s\n", resultucard);

    return 0;
}
