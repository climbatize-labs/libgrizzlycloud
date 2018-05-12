#include <gc.h>

inline static void init_filename(char *f, int nf, snb *pid)
{
    snb hexstr;

    bin2hexstr(&hexstr, pid);
    snprintf(f, nf, "/tmp/gc_%.*s",
                    sn_p(hexstr));
}

void fs_pair(struct hm_log_s *log, struct gc_device_pair_s *pair)
{
    char filename[128];
    char content[128];

    snb pid;
    snb_cpy_ds(pid, pair->pid);
    init_filename(filename, sizeof(filename), &pid);

    snprintf(content, sizeof(content), "%.*s\n%.*s",
                                       sn_p(pair->port_remote),
                                       sn_p(pair->port_local));

    if(gc_fwrite(filename, "w", content, strlen(content)) != 0) {
        hm_log(LOG_WARNING, log, "File [%s] couldn't be saved",
                                 filename);
    }
}

void fs_unpair(struct hm_log_s *log, snb *pid)
{
    char filename[128];

    init_filename(filename, sizeof(filename), pid);

    if(gc_fremove(filename) != 0) {
        hm_log(LOG_WARNING, log, "File [%s] couldn't be deleted",
                                 filename);
    }
}
