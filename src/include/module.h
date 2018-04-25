#ifndef GC_MODULE_H_
#define GC_MODULE_H_

#define MAX_MODULES   1

#define MOD_PHILLIPSHUE_PORT 10220

enum gc_module_e {
    MOD_NONE           = 0,
    MOD_PHILLIPSHUE    = (1 << 0)
};

struct gc_module_s {
    enum gc_module_e       id;
    const char             *name;
    int                    port;
    struct gc_gen_server_s *server;
    sn                     buffer;

    int (*start)(struct gc_s *gc, struct gc_module_s *module);
    int (*status)();
    int (*stop)(struct gc_s *gc, struct gc_module_s *module);
};

extern struct gc_module_s *modules_available[MAX_MODULES];

#endif
