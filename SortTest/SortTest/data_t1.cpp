#include "data_t.h"




inline void setKey(data_t & a, cl_uint key) { a.s[0] = key; }
inline void setValue(data_t & a, cl_uint value) { a.s[1] = value; }
inline cl_uint getKey(const data_t & a) { return a.s[0]; }
inline cl_uint getValue(const data_t & a) { return a.s[1]; }

inline bool operator < (const data_t & a, const data_t & b) { return getKey(a) < getKey(b); }
inline bool operator == (const data_t & a, const data_t & b) { return (getKey(a) == getKey(b)) && (getValue(a) == getValue(b)); }
inline bool operator != (const data_t & a, const data_t & b) { return (getKey(a) != getKey(b)) || (getValue(a) != getValue(b)); }
