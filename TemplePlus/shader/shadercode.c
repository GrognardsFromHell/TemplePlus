
#define BYTE unsigned char

#ifdef NDEBUG
#  include "../Release/clipping_ps.h"
#  include "../Release/clipping_vs.h"
#else
#  include "../Debug/clipping_ps.h"
#  include "../Debug/clipping_vs.h"
#endif

const unsigned clipping_vs_size = sizeof(clipping_vs);
const unsigned clipping_ps_size = sizeof(clipping_ps);
