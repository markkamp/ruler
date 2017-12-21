#ifndef TEST_H__
#define TEST_H__

extern void CompareAndMatchStrings(const char *mcardSrc, const size_t mcardSrcLen,
                                   const char *ucardSrc, const size_t ucardSrcLen,
                                   char *mcardDst, const size_t mcardDstLen,
                                   char *ucardDst, const size_t ucardDstLen,
                                   const uint32_t minCheckLen);

#endif /* TEST_H__ */
