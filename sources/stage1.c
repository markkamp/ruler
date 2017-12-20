#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include "stage1.h"
#include "generic.h"

static int32_t FindNextMatchIndex(const char str[], size_t strLen, const uint32_t startIndex);
static int32_t FindNextMatchIndexOfCheckLength(const char str[], size_t strLen,
                                               const uint32_t startIndex, const size_t checkLen);
static int32_t Compare(const char mcardDst[], size_t mcardDstLen,
                       const int32_t mcardDstStartIndex,
                       const char ucardDst[], size_t ucardDstLen,
                       const int32_t ucardDstStartIndex,
                       const size_t checkLen);
static void SetMatchedChars(char mcardDst[], size_t mcardDstLen,
                            const int32_t mcardDstStartIndex,
                            char ucardDst[], size_t ucardDstLen,
                            const int32_t ucardDstStartIndex,
                            const size_t checkLen);
static bool MatchStringsOfLength(char mcardDst[], size_t mcardDstLen,
                                 char ucardDst[], size_t ucardDstLen,
                                 const size_t checkLen);

/*!
 * \brief Finds the next match character and returns the match character index.
 * \param str
 *      The string to find the next match character in.
 * \param strLen
 *      The length of the string (excluding the NULL terminator).
 * \param startIndex
 *      The index in the string from which the start searching for the next match character.
 * \returns The next match character index or -1 if the end of the string was reached.
 */
static int32_t FindNextMatchIndex(const char str[], size_t strLen, const uint32_t startIndex)
{
    uint32_t i = 0;
    int32_t retVal = -1;

    assert(str != NULL);
    assert(strLen > 0);

    for (i = startIndex; i < strLen; i++) {
        if (IsMatchChar(str[i]) == true) {
            retVal = i;
            break;
        }
    }

    return retVal;
}

/*!
 * \brief Finds the next match string with minimum check length and returns the index.
 * \param str
 *      The string to find the next match string in.
 * \param strLen
 *      The length of the string (excluding the NULL terminator).
 * \param startIndex
 *      The index in the string from which the start searching for the next match character.
 * \param checkLen
 *      The length of the string to find.
 * \returns
 *      The next match character index of a string with minimum check length or -1 if the end of the
 *      string was reached.
 */
static int32_t FindNextMatchIndexOfCheckLength(const char str[], size_t strLen,
                                               const uint32_t startIndex, const size_t checkLen)
{
    uint32_t i = 0;
    uint32_t matchLength = 0;
    uint32_t newStartIndex = 0;
    int32_t retVal = -1;

    assert(str != NULL);
    assert(checkLen > 0);
    assert(strLen > 0);

    newStartIndex = startIndex;
    for (i = newStartIndex; i < strLen; i++) {
        if (IsMatchChar(str[i]) == true) {
            matchLength++;

            if (matchLength == checkLen) {
                /* Found, so break. */
                retVal = newStartIndex;
                break;
            }
        } else {
            /* Continue searching, reset matched length and update the start index. */
            matchLength = 0;
            newStartIndex = i + 1;
        }
    }

    return retVal;
}

static int32_t Compare(const char mcardDst[], size_t mcardDstLen,
                       const int32_t mcardDstStartIndex,
                       const char ucardDst[], size_t ucardDstLen,
                       const int32_t ucardDstStartIndex,
                       const size_t checkLen)
{
    int32_t tmpStartIndexGood = 0;
    int32_t tmpStartIndexCheck = 0;
    uint32_t matchedChars = 0;
    int32_t newUcardDstStartIndex = 0;

    assert(mcardDst != NULL);
    assert(ucardDst != NULL);
    assert(mcardDstLen > 0);
    assert(ucardDstLen > 0);
    assert((mcardDstLen - mcardDstStartIndex) <= mcardDstLen);
    assert((ucardDstLen - ucardDstStartIndex) <= ucardDstLen);
    assert(checkLen > 0);

    tmpStartIndexGood = mcardDstStartIndex;
    tmpStartIndexCheck = ucardDstStartIndex;
    while (matchedChars < checkLen) {
        if (mcardDst[tmpStartIndexGood] == ucardDst[tmpStartIndexCheck]) {
            /* Another character was matched, keep on matching. */
            matchedChars++;
            tmpStartIndexGood++;

            if (matchedChars == 1) {
                newUcardDstStartIndex = tmpStartIndexCheck;
            }

            tmpStartIndexGood = FindNextMatchIndex(mcardDst, mcardDstLen,
                                                   (uint32_t)tmpStartIndexGood);
            if (tmpStartIndexGood == -1) {
                break;
            }
        } else {
            /* A mismatch occurred before checkLen was reached.
             * Reset mcardDst and matchedChars.
             */
            tmpStartIndexGood = mcardDstStartIndex;
            matchedChars = 0;
        }

        tmpStartIndexCheck++;
        tmpStartIndexCheck = FindNextMatchIndex(ucardDst, ucardDstLen,
                                                (uint32_t)tmpStartIndexCheck);
        if (tmpStartIndexCheck == -1) {
            break;
        }
    }

    if (matchedChars != checkLen) {
        newUcardDstStartIndex = -1;
    }

    return newUcardDstStartIndex;
}

static void SetMatchedChars(char mcardDst[], size_t mcardDstLen,
                            const int32_t mcardDstStartIndex,
                            char ucardDst[], size_t ucardDstLen,
                            const int32_t ucardDstStartIndex,
                            const size_t checkLen)
{
    uint32_t i = 0;
    int32_t tmpMcardStartIndex = 0;
    int32_t tmpStartIndexCheck = 0;

    assert(mcardDst != NULL);
    assert(ucardDst != NULL);
    assert(mcardDstLen > 0);
    assert(ucardDstLen > 0);
    assert((mcardDstLen - mcardDstStartIndex) <= mcardDstLen);
    assert((ucardDstLen - ucardDstStartIndex) <= ucardDstLen);
    assert(checkLen > 0);

    tmpMcardStartIndex = mcardDstStartIndex;
    tmpStartIndexCheck = ucardDstStartIndex;

    /* This should always be true. */
    assert(mcardDst[tmpMcardStartIndex] == ucardDst[tmpStartIndexCheck]);
    mcardDst[tmpMcardStartIndex] = CHAR_FOUND;
    ucardDst[tmpStartIndexCheck] = CHAR_FOUND;

    for (i = 0; i < (checkLen - 1); i++) {
        tmpMcardStartIndex = FindNextMatchIndex(mcardDst, mcardDstLen,
                                                (uint32_t)tmpMcardStartIndex);
        assert(tmpStartIndexCheck != -1);

        tmpStartIndexCheck = FindNextMatchIndex(ucardDst, ucardDstLen,
                                                (uint32_t)tmpStartIndexCheck);
        assert(tmpStartIndexCheck != -1);

        /* Set the characters to found. */
        mcardDst[tmpMcardStartIndex] = CHAR_FOUND;
        ucardDst[tmpStartIndexCheck] = CHAR_FOUND;
    }
}

static bool MatchStringsOfLength(char mcardDst[], size_t mcardDstLen,
                                 char ucardDst[], size_t ucardDstLen,
                                 const size_t checkLen)
{
    bool foundMatch = false;
    int32_t mcardDstStartIndex = 0;
    int32_t ucardDstStartIndex = 0;
    int32_t newUcardDstStartIndex = 0;
    int32_t nextStartIndexGood = 0;
    size_t len = MIN(mcardDstLen, ucardDstLen);

    assert(checkLen <= len);

    while (nextStartIndexGood <= len) {
        /* Find first valid character in good. */
        assert(nextStartIndexGood != -1);
        mcardDstStartIndex = FindNextMatchIndexOfCheckLength(mcardDst, mcardDstLen,
                                                             (uint32_t)nextStartIndexGood,
                                                             checkLen);
        if (mcardDstStartIndex == -1) {
            break;
        }

        /* Find first valid character in check. */
        ucardDstStartIndex = FindNextMatchIndexOfCheckLength(ucardDst, ucardDstLen, 0, checkLen);
        if (ucardDstStartIndex == -1) {
            break;
        }

        /* Compare. */
        newUcardDstStartIndex = Compare(mcardDst, mcardDstLen, mcardDstStartIndex,
                                        ucardDst, ucardDstLen, ucardDstStartIndex,
                                        checkLen);

        if (newUcardDstStartIndex != -1) {
            /* Match found, set matched characters to CHAR_FOUND. */
            SetMatchedChars(mcardDst, mcardDstLen, mcardDstStartIndex,
                            ucardDst, ucardDstLen, newUcardDstStartIndex,
                            checkLen);
            /* Set the return code to true, we did find at least 1 match. */
            foundMatch = true;

            nextStartIndexGood += checkLen;
        } else {
            assert((mcardDstStartIndex + 1) >= 0);
            nextStartIndexGood = (uint32_t)(mcardDstStartIndex + 1);
        }
    }

    return foundMatch;
}

void MatchStrings(const char mcardSrc[], const size_t mcardSrcLen,
                  const char ucardSrc[], const size_t ucardSrcLen,
                  char mcardDst[], const size_t mcardDstLen,
                  char ucardDst[], const size_t ucardDstLen,
                  const uint32_t minCheckLen)
{
    size_t len = 0;
    size_t mcardResStrippedLen = 0;
    size_t ucardResStrippedLen = 0;

    assert(mcardSrc != NULL);
    assert(ucardSrc != NULL);
    assert(mcardDst != NULL);
    assert(ucardDst != NULL);
    assert(mcardSrcLen > 0);
    assert(ucardSrcLen > 0);
    assert(mcardDstLen > 0);
    assert(ucardDstLen > 0);
    assert(minCheckLen > 0);

    /* Strip any characters other than a-z and 0-9 and write into the result array. */
    mcardResStrippedLen = StripIgnoreChars(mcardSrc, mcardSrcLen, mcardDst, mcardDstLen);
    ucardResStrippedLen = StripIgnoreChars(ucardSrc, ucardSrcLen, ucardDst, ucardDstLen);

    /* Match from the largest possible common string down to and including the minCheckLen. */
    for (len = MIN(mcardResStrippedLen, ucardResStrippedLen); len >= minCheckLen; len--) {
        if (MatchStringsOfLength(mcardDst, mcardResStrippedLen,
                                 ucardDst, ucardResStrippedLen,
                                 len) == true) {
            /* Print the new results. */
            printf("\nStage 1: len %u\n", (uint32_t)len);

            printf("         mcard: %s\n", mcardDst);
            printf("         ucard: %s\n", ucardDst);
        }
    }

    /* Expand the result to include the previously stripped ignored characters. */
    //TODO
}
