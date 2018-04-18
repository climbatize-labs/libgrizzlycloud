#include <gc.h>

extern struct gc_module_s module_phillipshue;

struct gc_module_s *modules_available[MAX_MODULES] = {
    &module_phillipshue
};
