#include <gc.h>

inline static void init_filename(char *f, int nf, snb *pid, sn port_remote)
{
    snb hexstr;

    bin2hexstr(&hexstr, pid);
    snprintf(f, nf, "/tmp/gc_%.*s_%.*s",
                    sn_p(hexstr), sn_p(port_remote));
}

void fs_pair(struct hm_log_s *log, struct gc_device_pair_s *pair)
{
    char filename[128];
    char content[256];

    snb pid;
    snb_cpy_ds(pid, pair->pid);
    init_filename(filename, sizeof(filename), &pid, pair->port_remote);
    printf("FS pair: %s\n", filename);

    snprintf(content, sizeof(content), "{\"cloud\"      : \"%.*s\",\n\
                                         \"device\"     : \"%.*s\",\n\
                                         \"portRemote\" : %.*s,\n\
                                         \"portLocal\"  : %.*s }",
                                       sn_p(pair->cloud),
                                       sn_p(pair->device),
                                       sn_p(pair->port_remote),
                                       sn_p(pair->port_local));

    if(gc_fwrite(filename, "w", content, strlen(content)) != 0) {
        hm_log(LOG_WARNING, log, "File [%s] couldn't be saved",
                                 filename);
    }
    printf("File %s saved\n", filename);
}

void fs_unpair(struct hm_log_s *log, snb *pid, int port_remote)
{
    char filename[128];

    sn_itoa(pr, port_remote, 32);

    init_filename(filename, sizeof(filename), pid, pr);
    printf("FS unpair: %s\n", filename);

    if(gc_fremove(filename) != 0) {
        hm_log(LOG_WARNING, log, "File [%s] couldn't be deleted",
                                 filename);
    }
}
