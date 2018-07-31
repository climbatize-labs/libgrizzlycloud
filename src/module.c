#include <gc.h>

extern struct gc_module_s module_phillipshue;
extern struct gc_module_s module_webcam;

struct gc_module_s *modules_available[MAX_MODULES] = {
    &module_phillipshue,
    &module_webcam
};
