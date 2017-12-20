#ifndef GENERIC_H__
#define GENERIC_H__

#define MIN(_a, _b)  ((_a) <= (_b) ? (_a) : (_b))
#define MAX(_a, _b)  ((_a) >= (_b) ? (_a) : (_b))

#define MAX_STR_LENGTH  100

#define CHAR_BAD        '*'
#define CHAR_IGNORED    ' '
#define CHAR_IGNORED1   '['
#define CHAR_IGNORED2   ']'
#define CHAR_FOUND      '_'

extern bool IsIgnoreChar(const char c);
extern bool IsFoundChar(const char c);
extern bool IsMatchChar(const char c);

extern size_t StripIgnoreChars(const char string[], const size_t stringLen,
                               char result[], const size_t resultLen);

#endif /* GENERIC_H__ */
