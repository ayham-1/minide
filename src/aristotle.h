#ifndef ARISTOTLE_H
#define ARISTOTLE_H

#define max(a, b) \
    ({__typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a, b) \
    ({__typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
     _a > _b ? _b : _a; })

#endif
