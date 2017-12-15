#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define MIN(_a, _b)  ((_a) <= (_b) ? (_a) : (_b))
#define MAX(_a, _b)  ((_a) >= (_b) ? (_a) : (_b))

#define MAX_STR_LENGTH  100

#define CHAR_BAD        '*'
#define CHAR_IGNORED    ' '
#define CHAR_IGNORED1   '['
#define CHAR_IGNORED2    ']'
#define CHAR_FOUND      '_'

static char *mcardGood = "1a [bkg]do[efe]fs[  ]gshnu[j ]ln[b ][w ]a[rv]p[ t]vxwayb[ cb]23bac deghe[ f]jkr[ ln]cmcoi stxuwyf4";
static char *mcardErr =  "  c               em        w     u   j        z         e        q      z      o        p        ";

//01a,01b
static char *ucard = "1abkgdofsgshnjlnbjarvzvxwaybcb23bacdeghefjkrlncmcoipstuwyf4";
//05a,05b
//static char *ucard = "1akdoefgmggnwlnbjapzvxwybe23bacdegfjklwmcoipstxuwyf4";

static char stripped[MAX_STR_LENGTH];
static char resultucard[MAX_STR_LENGTH];

static char mcardGoodStage1[MAX_STR_LENGTH];
static char ucardStage1[MAX_STR_LENGTH];

#ifdef MINGW
static size_t strnlen (const char *string, size_t maxlen);
#endif
static bool IsIgnoreChar(const char c);
static bool IsFoundChar(const char c);
static bool IsMatchChar(const char c);
static void StripIgnoreChars(const char string[], char resultArray[], const size_t resultArraySize);
static int32_t FindNextMatchIndex(const char string[], const int32_t fromIndex);
static int32_t FindNextMatchIndexOfCheckLength(const char string[], const uint32_t fromIndex,
                                               const uint32_t checkLength);
static int32_t Compare(const char goodString[], const int32_t startIndexGood,
                       const char checkString[], const int32_t startIndexCheck,
                       const uint32_t checkLength);
static bool FindMatchInString(char goodString[], char checkString[], const uint32_t checkLength);

#ifdef MINGW
/* Required for MINGW. */
static size_t strnlen (const char *string, size_t maxlen)
{
    const char *end = memchr (string, '\0', maxlen);

    return end ? (size_t) (end - string) : maxlen;
}
#endif

static bool IsIgnoreChar(const char c)
{
    if ((c == CHAR_IGNORED) ||
        (c == CHAR_IGNORED1) ||
        (c == CHAR_IGNORED2)) {
        return true;
    }

    return false;
}

static bool IsFoundChar(const char c)
{
    if (c == CHAR_FOUND) {
        return true;
    }

    return false;
}

static bool IsMatchChar(const char c)
{
    if (((c >= 'a') && (c <= 'z')) ||
        ((c >= 'A') && (c <= 'Z')) ||
        ((c >= '0') && (c <= '9'))) {
        return true;
    }

    return false;
}

static void StripIgnoreChars(const char string[], char resultArray[], const size_t resultArraySize)
{
    uint32_t i;
    uint32_t j;

    assert(string != 0);
    assert(resultArray != 0);

    for (i = 0, j = 0; (i < strnlen(string, MAX_STR_LENGTH)) && (j < resultArraySize); i++) {
        if (IsIgnoreChar(string[i]) == false) {
            resultArray[j] = string[i];
            j++;
        }
    }

    resultArray[MIN(j, resultArraySize)] = '\0';
}

/*!
 * \brief Finds the next match character and returns the index or -1 if end of string.
 */
static int32_t FindNextMatchIndex(const char string[], const int32_t fromIndex)
{
    int32_t i;

    /* Find first valid character in good. */
    for (i = fromIndex; i < strnlen(string, MAX_STR_LENGTH); i++) {
        if (IsMatchChar(string[i]) == true) {
            return i;
        }
    }

    return -1;
}

/*!
 * \brief Finds the next match character of a string with minimum compare length and returns the
 *        index or -1 if end of string.
 *        May include ignore characters, but not found characters.
 */
static int32_t FindNextMatchIndexOfCheckLength(const char string[], const uint32_t fromIndex,
                                               const uint32_t checkLength)
{
    int32_t i;
    int32_t newFromIndex = 0;
    int32_t matchLength = 0;
    int32_t index = fromIndex;
    size_t strLen = strnlen(string, MAX_STR_LENGTH);

    newFromIndex = fromIndex;
    while (index < strLen) {
        /* Compare all upcoming characters to be a valid character. */
        for (i = newFromIndex; (i < strLen) && (matchLength < checkLength); i++) {
            if (IsMatchChar(string[i]) == true) {
                matchLength++;
            } else if (IsFoundChar(string[i]) == true) {
                /* A previously matched character breaks the continues string so stop here. */
                break;
            } /* Else continue searching. */
        }

        if (matchLength != checkLength) {
            index++;
            //newFromIndex = -1;
        } else {
            break;
        }

        /* Find next valid character. */
        newFromIndex = FindNextMatchIndex(string, index);
        if (newFromIndex != -1) {
            break;
        }
    }

    return newFromIndex;
}

static int32_t Compare(const char goodString[], const int32_t startIndexGood,
                       const char checkString[], const int32_t startIndexCheck,
                       const uint32_t checkLength)
{
    int32_t tmpStartIndexGood = startIndexGood;
    int32_t tmpStartIndexCheck = startIndexCheck;
    uint32_t matchedChars = 0;
    int32_t matchedStartIndexCheck = 0;

    /* Compare. */
    while (matchedChars < checkLength) {
        if (goodString[tmpStartIndexGood] == checkString[tmpStartIndexCheck]) {
            /* Another character was matched, keep on matching. */
            matchedChars++;
            tmpStartIndexGood++;

            if (matchedChars == 1) {
                matchedStartIndexCheck = tmpStartIndexCheck;
            }

            tmpStartIndexGood = FindNextMatchIndex(goodString, tmpStartIndexGood);
            if (tmpStartIndexGood == -1) {
                break;
            }
        } else {
            /* A mismatch occured before checkLength was reached.
             * Reset goodString and matchedChars.
             */
            tmpStartIndexGood = startIndexGood;
            matchedChars = 0;
        }

        tmpStartIndexCheck++;
        tmpStartIndexCheck = FindNextMatchIndex(checkString, tmpStartIndexCheck);
        if (tmpStartIndexCheck == -1) {
            break;
        }
    }

    if (matchedChars != checkLength) {
        matchedStartIndexCheck = -1;
    }

    return matchedStartIndexCheck;

}

static int32_t SetMatchedChars(char goodString[], const int32_t startIndexGood,
                               char checkString[], const int32_t startIndexCheck,
                               const uint32_t checkLength)
{
    uint32_t i = 0;
    int32_t tmpStartIndexGood = startIndexGood;
    int32_t tmpStartIndexCheck = startIndexCheck;
    int32_t nextStartIndexGood = -1;

    /* This should always be true. */
    assert(goodString[tmpStartIndexGood] == checkString[tmpStartIndexCheck]);

    /* This should always be true. */
    goodString[tmpStartIndexGood] = CHAR_FOUND;
    checkString[tmpStartIndexCheck] = CHAR_FOUND;

    for (i = 0; i < (checkLength - 1); i++) {
        assert(tmpStartIndexGood >= 0);
        nextStartIndexGood = (uint32_t)tmpStartIndexGood;

        tmpStartIndexGood = FindNextMatchIndex(goodString, tmpStartIndexGood);
        if (tmpStartIndexGood == -1) {
            /* This should never happen. */
            assert(false);
        }

        tmpStartIndexCheck = FindNextMatchIndex(checkString, tmpStartIndexCheck);
        if (tmpStartIndexCheck == -1) {
            /* This should never happen. */
            assert(false);
        }

        /* This should always be true. */
        assert(goodString[tmpStartIndexGood] == checkString[tmpStartIndexCheck]);

        /* Set the characters to found. */
        goodString[tmpStartIndexGood] = CHAR_FOUND;
        checkString[tmpStartIndexCheck] = CHAR_FOUND;
    }

    return nextStartIndexGood;
}

static bool FindMatchInString(char goodString[], char checkString[], const uint32_t checkLength)
{
    bool foundMatch = false;
    int32_t startIndexGood = 0;
    int32_t startIndexCheck = 0;
    int32_t matchedStartIndexCheck = 0;
    int32_t nextStartIndexGood = 0;
    size_t strLen = strnlen(goodString, MAX_STR_LENGTH);

    assert(checkLength <= strLen);

    while (nextStartIndexGood <= (strLen - checkLength)) {
        /* Find first valid character in good. */
        assert(nextStartIndexGood != -1);
        startIndexGood = FindNextMatchIndexOfCheckLength(goodString, (uint32_t)nextStartIndexGood, checkLength);
        if (startIndexGood == -1) {
            break;
        }

        /* Find first valid character in check. */
        startIndexCheck = FindNextMatchIndexOfCheckLength(checkString, 0, checkLength);
        if (startIndexCheck == -1) {
            break;
        }

        /* Compare. */
        matchedStartIndexCheck = Compare(goodString, startIndexGood,
                                         checkString, startIndexCheck,
                                         checkLength);

        if (matchedStartIndexCheck != -1) {
            /* Match found, set matched characters to CHAR_FOUND. */
            nextStartIndexGood = SetMatchedChars(goodString, startIndexGood,
                                                 checkString, matchedStartIndexCheck,
                                                 checkLength);
            assert(nextStartIndexGood != -1);

            /* Set the return code to true, we did find at least 1 match. */
            foundMatch = true;
        } else {
            assert((startIndexGood + 1) >= 0);
            nextStartIndexGood = (uint32_t)(startIndexGood + 1);
        }
    }

    return foundMatch;
}

int main(void)
{
    uint32_t len;

    printf("\nInput:\n");
    printf("     mcardgood: %s\n", mcardGood);
    printf("      mcarderr: %s\n", mcardErr);
    printf("         ucard: %s\n", ucard);

    /* Scan input strings for validity. */
    //TODO

    /* Prepare arrays for stage 1. */
    assert(strnlen(mcardGood, MAX_STR_LENGTH) < MAX_STR_LENGTH);
    memcpy(mcardGoodStage1, mcardGood, strnlen(mcardGood, MAX_STR_LENGTH) + 1);
    assert(strnlen(ucard, MAX_STR_LENGTH) < MAX_STR_LENGTH);
    memcpy(ucardStage1, ucard, strnlen(ucard, MAX_STR_LENGTH) + 1);

    /* Perform stage 1: the removal of matched string from max length down to length 3. */
    for (len = (uint32_t)strnlen(mcardGood, MAX_STR_LENGTH); len >= 3; len--) {
        if (FindMatchInString(mcardGoodStage1, ucardStage1, len) == true) {
            /* Print the new results. */
            printf("\nStage 1: len %d\n", len);

            StripIgnoreChars(mcardGoodStage1, stripped, sizeof(stripped));
            printf("         mcard: %s\n", stripped);
            StripIgnoreChars(ucardStage1, stripped, sizeof(stripped));
            printf("         ucard: %s\n", stripped);
        }
    }

    /* Stage 2. */
    //TODO

    /* Print results. */
    printf("\nResults:\n");

    StripIgnoreChars(mcardGood, stripped, sizeof(stripped));
    printf(" str mcardgood: %s\n", stripped);

    printf("         ucard: %s\n", ucard);
    printf("   resultucard: %s\n", resultucard);

    return 0;
}






