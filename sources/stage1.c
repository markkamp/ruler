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
                       const int32_t ucardDestStartIndex,
                       const size_t checkLen);
static int32_t SetMatchedChars(char mcardDst[], size_t mcardDstLen,
                               const int32_t mcardDstStartIndex,
                               char ucardDst[], size_t ucardDstLen,
                               const int32_t ucardDestStartIndex,
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
    uint32_t i;

    assert(str != NULL);
    assert(strLen > 0);
    assert(startIndex >= 0);

    /* Find first valid character in good. */
    for (i = startIndex; i < strLen; i++) {
        if (IsMatchChar(str[i]) == true) {
            return i;
        }
    }

    return -1;
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
    int32_t newStartIndex = 0;
    uint32_t matchLength = 0;

    assert(str != NULL);
    assert(strLen > 0);
    assert(startIndex >= 0);
    assert((strLen - startIndex) <= strLen);
    assert(checkLen > 0);

    newStartIndex = startIndex;
    while (newStartIndex < strLen) {
        /* Compare all upcoming characters to be a valid character. */
        for (i = (uint32_t)newStartIndex; i < strLen; i++) {
            if (IsMatchChar(str[i]) == true) {
                matchLength++;

                if (matchLength == checkLen) {
                    /* Found, so break. */
                    break;
                }
            } else {
                /* A previously matched character breaks the continues str so stop here. */
                break;
            } /* Else continue searching. */
        }

        if (matchLength == checkLen) {
            /* Found, so break. */
            break;
        }

        /* Find next valid character. */
        newStartIndex++;
        matchLength = 0;
        newStartIndex = FindNextMatchIndex(str, strLen, (uint32_t)newStartIndex);
        if (newStartIndex == -1) {
            break;
        }
    }

    return newStartIndex;
}

static int32_t Compare(const char mcardDst[], size_t mcardDstLen,
                       const int32_t mcardDstStartIndex,
                       const char ucardDst[], size_t ucardDstLen,
                       const int32_t ucardDestStartIndex,
                       const size_t checkLen)
{
    int32_t tmpStartIndexGood = mcardDstStartIndex;
    int32_t tmpStartIndexCheck = ucardDestStartIndex;
    uint32_t matchedChars = 0;
    int32_t ucardDstStartIndex = 0;

    /* Compare. */
    while (matchedChars < checkLen) {
        if (mcardDst[tmpStartIndexGood] == ucardDst[tmpStartIndexCheck]) {
            /* Another character was matched, keep on matching. */
            matchedChars++;
            tmpStartIndexGood++;

            if (matchedChars == 1) {
                ucardDstStartIndex = tmpStartIndexCheck;
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
        ucardDstStartIndex = -1;
    }

    return ucardDstStartIndex;
}

static int32_t SetMatchedChars(char mcardDst[], size_t mcardDstLen,
                               const int32_t mcardDstStartIndex,
                               char ucardDst[], size_t ucardDstLen,
                               const int32_t ucardDestStartIndex,
                               const size_t checkLen)
{
    uint32_t i = 0;
    int32_t tmpStartIndexGood = mcardDstStartIndex;
    int32_t tmpStartIndexCheck = ucardDestStartIndex;
    int32_t nextStartIndexGood = -1;

    /* This should always be true. */
    assert(mcardDst[tmpStartIndexGood] == ucardDst[tmpStartIndexCheck]);
    mcardDst[tmpStartIndexGood] = CHAR_FOUND;
    ucardDst[tmpStartIndexCheck] = CHAR_FOUND;

    for (i = 0; i < (checkLen - 1); i++) {
        nextStartIndexGood = (uint32_t)tmpStartIndexGood;

        tmpStartIndexGood = FindNextMatchIndex(mcardDst, mcardDstLen,
                                               (uint32_t)tmpStartIndexGood);
        assert(tmpStartIndexCheck != -1);

        tmpStartIndexCheck = FindNextMatchIndex(ucardDst, ucardDstLen,
                                                (uint32_t)tmpStartIndexCheck);
        assert(tmpStartIndexCheck != -1);

        /* Set the characters to found. */
        mcardDst[tmpStartIndexGood] = CHAR_FOUND;
        ucardDst[tmpStartIndexCheck] = CHAR_FOUND;
    }

    return nextStartIndexGood;
}

static bool MatchStringsOfLength(char mcardDst[], size_t mcardDstLen,
                                 char ucardDst[], size_t ucardDstLen,
                                 const size_t checkLen)
{
    bool foundMatch = false;
    int32_t mcardDstStartIndex = 0;
    int32_t ucardDestStartIndex = 0;
    int32_t ucardDstStartIndex = 0;
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
        ucardDestStartIndex = FindNextMatchIndexOfCheckLength(ucardDst, ucardDstLen, 0, checkLen);
        if (ucardDestStartIndex == -1) {
            break;
        }

        /* Compare. */
        ucardDstStartIndex = Compare(mcardDst, mcardDstLen, mcardDstStartIndex,
                                     ucardDst, ucardDstLen, ucardDestStartIndex,
                                     checkLen);

        if (ucardDstStartIndex != -1) {
            /* Match found, set matched characters to CHAR_FOUND. */
            nextStartIndexGood = SetMatchedChars(mcardDst, mcardDstLen, mcardDstStartIndex,
                                                 ucardDst, ucardDstLen, ucardDstStartIndex,
                                                 checkLen);
            /* Set the return code to true, we did find at least 1 match. */
            foundMatch = true;

            if (nextStartIndexGood == -1) {
                break;
            }
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
