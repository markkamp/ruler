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
static int32_t Match(char *mcardDst, size_t mcardDstLen,
                     const int32_t mcardDstStartIndex,
                     char *ucardDst, size_t ucardDstLen,
                     const int32_t ucardDstStartIndex,
                     const size_t checkLen);
static bool CompareAndMatchStringsOfLength(char *mcardDst, size_t mcardDstLen,
                                           char *ucardDst, size_t ucardDstLen,
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

/*!
 * \brief Compare the mcard and ucard strings for a match of checkLen length.
 * \details
 *      The mcard and ucard starting indices should have match characters to start with and
 *      should have a matching length of checkLen at a minimum.
 *      Each failed iteration the ucard starting index is offset by 1, the mcard starting index is
 *      reset the the original value.
 * \param mcardDst
 *      The mcard destination array.
 * \param mcardDstLen
 *      The mcard array length.
 * \param mcardDstStartIndex
 *      The mcard destination start index.
 * \param mcardDst
 *      The ucard destination array.
 * \param ucardDstLen
 *      The ucard array length.
 * \param ucardDstStartIndex
 *      The ucard destination start index.
 * \param checkLen
 *      The length of the string to compare.
 * \returns The ucard starting index of the beginning of the matched string or -1 if not found.
 */
static int32_t Compare(const char mcardDst[], size_t mcardDstLen,
                       const int32_t mcardDstStartIndex,
                       const char ucardDst[], size_t ucardDstLen,
                       const int32_t ucardDstStartIndex,
                       const size_t checkLen)
{
    int32_t tmpMcardDstStartIndex = 0;
    int32_t tmpUcardDstStartIndex = 0;
    uint32_t matchedChars = 0;
    int32_t newUcardDstStartIndex = -1;
    int32_t add = 0;

    assert(mcardDst != NULL);
    assert(ucardDst != NULL);
    assert(mcardDstLen > 0);
    assert(ucardDstLen > 0);
    assert((mcardDstLen - mcardDstStartIndex) <= mcardDstLen);
    assert((ucardDstLen - ucardDstStartIndex) <= ucardDstLen);
    assert(checkLen > 0);
    assert(IsMatchChar(mcardDst[mcardDstStartIndex]) == true);
    assert(IsMatchChar(ucardDst[ucardDstStartIndex]) == true);

    tmpMcardDstStartIndex = mcardDstStartIndex;
    tmpUcardDstStartIndex = ucardDstStartIndex;
    add = tmpUcardDstStartIndex;

    while (matchedChars < checkLen) {
        if (mcardDst[tmpMcardDstStartIndex] == ucardDst[tmpUcardDstStartIndex]) {
            /* Another character was matched, keep on matching. */
            matchedChars++;
            tmpMcardDstStartIndex++;

            if (matchedChars == 1) {
                newUcardDstStartIndex = tmpUcardDstStartIndex;
            }

            tmpMcardDstStartIndex = FindNextMatchIndex(mcardDst, mcardDstLen,
                                                       (uint32_t)tmpMcardDstStartIndex);
            if (tmpMcardDstStartIndex == -1) {
                break;
            }

            tmpUcardDstStartIndex++;
            tmpUcardDstStartIndex = FindNextMatchIndex(ucardDst, ucardDstLen,
                                                       (uint32_t)tmpUcardDstStartIndex);
            if (tmpUcardDstStartIndex == -1) {
                break;
            }
        } else {
            /* A mismatch occurred before checkLen was reached.
             * Reset mcardDst start index and matchedChars to the original values.
             * Increase ucardDst matching index by 1 from the original value.
             */
            tmpMcardDstStartIndex = mcardDstStartIndex;
            matchedChars = 0;
            add++;

            tmpUcardDstStartIndex = add;
            tmpUcardDstStartIndex = FindNextMatchIndex(ucardDst, ucardDstLen,
                                                       (uint32_t)tmpUcardDstStartIndex);
            if (tmpUcardDstStartIndex == -1) {
                break;
            }
        }
    }

    if (matchedChars != checkLen) {
        newUcardDstStartIndex = -1;
    }

    return newUcardDstStartIndex;
}

/*!
 * \brief
 *      Set the matched strings in the mcard and ucard to the match character starting at the given
 *      indices.
 * \details This will ignore/skip over any previously matched characters.
 * \param[in,out] mcardDst
 *      The mcard destination array.
 * \param mcardDstLen
 *      The mcard array length.
 * \param mcardDstStartIndex
 *      The mcard destination start index.
 * \param[in,out] ucardDst
 *      The ucard destination array.
 * \param ucardDstLen
 *      The ucard array length.
 * \param ucardDstStartIndex
 *      The ucard destination start index.
 * \param checkLen
 *      The length of the string to set.
 * \returns The next mcard matching index or -1 if the end has been reached.
 */
static int32_t Match(char *mcardDst, size_t mcardDstLen,
                     const int32_t mcardDstStartIndex,
                     char *ucardDst, size_t ucardDstLen,
                     const int32_t ucardDstStartIndex,
                     const size_t checkLen)
{
    uint32_t matched = 0;
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

    do {
        assert(tmpMcardStartIndex != -1);
        assert(tmpStartIndexCheck != -1);

        /* Set the characters to found. */
        mcardDst[tmpMcardStartIndex] = CHAR_FOUND;
        ucardDst[tmpStartIndexCheck] = CHAR_FOUND;

        tmpMcardStartIndex = FindNextMatchIndex(mcardDst, mcardDstLen,
                                                (uint32_t)tmpMcardStartIndex);
        tmpStartIndexCheck = FindNextMatchIndex(ucardDst, ucardDstLen,
                                                (uint32_t)tmpStartIndexCheck);
    } while (++matched < checkLen);

    return tmpMcardStartIndex;
}

/*!
 * \brief Match the mcard and ucard strings and set any matched characters to the match character.
 * \param[in,out] mcardDst
 *      The mcard destination array.
 * \param mcardDstLen
 *      The mcard array length.
 * \param[in,out] ucardDst
 *      The ucard destination array.
 * \param ucardDstLen
 *      The ucard array length.
 * \param checkLen
 *      The length of the string to match.
 * \returns Boolean value indicating if at least 1 match was found/set.
 */
static bool CompareAndMatchStringsOfLength(char *mcardDst, size_t mcardDstLen,
                                           char *ucardDst, size_t ucardDstLen,
                                           const size_t checkLen)
{
    bool foundMatch = false;
    int32_t mcardDstStartIndex = 0;
    int32_t ucardDstStartIndex = 0;
    int32_t tmpUcardDstStartIndex = 0;

    assert(mcardDst != NULL);
    assert(ucardDst != NULL);
    assert(mcardDstLen > 0);
    assert(ucardDstLen > 0);
    assert((mcardDstLen - checkLen) <= mcardDstLen);
    assert((ucardDstLen - checkLen) <= ucardDstLen);
    assert(checkLen > 0);

    /* Find first valid character of a match string with checkLen in mcardDst. */
    mcardDstStartIndex = FindNextMatchIndexOfCheckLength(mcardDst, mcardDstLen,
                                                         (uint32_t)mcardDstStartIndex,
                                                         checkLen);

    while (mcardDstStartIndex != -1) {
        /* Find first valid character of a match string with checkLen in ucardDst. */
        ucardDstStartIndex = FindNextMatchIndexOfCheckLength(ucardDst, ucardDstLen, 0, checkLen);
        if (ucardDstStartIndex == -1) {
            break;
        }

        /* Compare until a match is found in uCardDst or until nothing to match. */
        tmpUcardDstStartIndex = Compare(mcardDst, mcardDstLen, mcardDstStartIndex,
                                        ucardDst, ucardDstLen, ucardDstStartIndex,
                                        checkLen);

        if (tmpUcardDstStartIndex != -1) {
            /* Match found, set matched characters to CHAR_FOUND.
             * The return value makes it very quick to start scanning from the next valid mcard
             * character.
             */
            mcardDstStartIndex = Match(mcardDst, mcardDstLen, mcardDstStartIndex,
                                       ucardDst, ucardDstLen, tmpUcardDstStartIndex,
                                       checkLen);

            /* Set the return code to true, we did find at least 1 match. */
            foundMatch = true;
        } else {
            /* Find first valid character of a match string with checkLen in mcardDst. */
            mcardDstStartIndex = FindNextMatchIndexOfCheckLength(mcardDst, mcardDstLen,
                                                                 (uint32_t)++mcardDstStartIndex,
                                                                 checkLen);
        }
    }

    return foundMatch;
}

void CompareAndMatchStrings(const char *mcardSrc, const size_t mcardSrcLen,
                            const char *ucardSrc, const size_t ucardSrcLen,
                            char *mcardDst, const size_t mcardDstLen,
                            char *ucardDst, const size_t ucardDstLen,
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
        if (CompareAndMatchStringsOfLength(mcardDst, mcardResStrippedLen,
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
