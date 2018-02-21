#ifndef HMLOG_H_
#define HMLOG_H_

enum errors_e {
    LOG_EMERG = 0,
    LOG_ALERT,
    LOG_CRIT,
    LOG_ERR,
    LOG_WARNING,
    LOG_NOTICE,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TRACE,
};

struct hm_log_s {
    const char *name;
    int fd;
    FILE *file;
    void *data;
    int priority;
};

#define hm_log(t, l, fmt...)\
    hm_log_impl(t, l, __FILE__, __LINE__, __FUNCTION__, fmt)

int hm_log_impl(int priority, struct hm_log_s *log, const char *file, int line, const char *func, const char *fmt, ...) __attribute__ ((format (printf, 6, 7)));
int hm_log_open(struct hm_log_s *l, const char *filename, const int priority);
int hm_log_close(struct hm_log_s *l);

#endif
