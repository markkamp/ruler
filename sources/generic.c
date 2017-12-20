#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "generic.h"

bool IsIgnoreChar(const char c)
{
    if ((c == CHAR_IGNORED) ||
        (c == CHAR_IGNORED1) ||
        (c == CHAR_IGNORED2)) {
        return true;
    }

    return false;
}

bool IsFoundChar(const char c)
{
    if (c == CHAR_FOUND) {
        return true;
    }

    return false;
}

bool IsMatchChar(const char c)
{
    if (((c >= 'a') && (c <= 'z')) ||
        ((c >= '0') && (c <= '9'))) {
        return true;
    }

    return false;
}

/*!
 * \brief
 * \param string
 * \param stringLen
 * \param result
 * \param resultLen
 * \returns The resulting string length.
 */
size_t StripIgnoreChars(const char string[], const size_t stringLen,
                        char result[], const size_t resultLen)
{
    uint32_t i;
    uint32_t j;

    assert(string != 0);
    assert(result != 0);

    for (i = 0, j = 0; (i < stringLen) && (j < resultLen); i++) {
        if (IsIgnoreChar(string[i]) == false) {
            result[j] = string[i];
            j++;
        }
    }

    result[MIN(j, resultLen)] = '\0';

    return j;
}