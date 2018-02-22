#ifndef UTILS_H_
#define UTILS_H_

#define MAX_FILE_SIZE   (1024 * 1024)

/*
 * Custom string SN
 * N characters starting at pointer S
 */

/*
 * Initialize m_dst with m_src
 */
#define sn_init(m_dst, m_src)\
    sn m_dst = { .s = m_src.s, .n = m_src.n, .offset = m_src.offset }

/*
 * Initialize SN with an external raw buffer and length
 */
#define sn_initr(m_dst, m_src_s, m_src_n)\
    sn m_dst = { .s = m_src_s, .n = m_src_n, .offset = 0 }

/*
 * Initialize SN with an external zero terminated buffer
 */
#define sn_initz(m_dst, m_src)\
    sn m_dst = { .s = m_src, .n = strlen(m_src), .offset = 0 }

/*
 * Set existing string m_var with string m_src
 */
#define sn_set(m_var, m_src)\
    m_var.s      = m_src.s;\
    m_var.n      = m_src.n;\
    m_var.offset = m_src.offset;

/*
 * Set existing string m_var with an external raw buffer and length
 */
#define sn_setr(m_var, m_src, m_src_n)\
    m_var.s      = m_src;\
    m_var.n      = m_src_n;\
    m_var.offset = 0;

/*
 * Set existing string m_var with an external zero terminated buffer
 */
#define sn_setz(m_var, m_src)\
    m_var.s      = m_src;\
    m_var.n      = strlen(m_src);\
    m_var.offset = 0;

/*
 * Format to printf()
 */
#define sn_p(m_var) m_var.n, m_var.s

/*
 * Allocate m_size bytes from heap
 *
 * See sn_bytes_append(), sn_bytes_delete()
 */
#define sn_bytes_new(m_var, m_size)\
    sn m_var = { .s = malloc(m_size), .n = m_size, .offset = 0 };\
    assert(m_var.s != NULL);

/*
 * Append m_src string to m_var string
 * Warning: this is not aligned
 *
 * See sn_bytes_new(), sn_bytes_delete()
 */
#define sn_bytes_append(m_var, m_src)\
    assert((m_var.offset + m_src.n) <= m_var.n);\
    memcpy(m_var.s + m_var.offset, m_src.s, m_src.n);\
    m_var.offset += m_src.n;

/*
 * Free allocated m_var allocated
 *
 * See sn_bytes_new(), sn_bytes_append()
 */
#define sn_bytes_delete(m_var) free(m_var.s)

/*
 * Copy string m_src to buffer of chars m_var with size m_size
 */
#define sn_to_char(m_var, m_src, m_size)\
    char buf##m_var[m_size];\
    snprintf(buf##m_var, sizeof(buf##m_var), "%.*s", m_src.n, m_src.s);\
    char *m_var = buf##m_var;

/*
 * Convert string m_src to int m_var
 * while m_size is conversion buffer size
 */
#define sn_atoi(m_var, m_src, m_size)\
    char buf##m_var[m_size];\
    snprintf(buf##m_var, sizeof(buf##m_var), "%.*s", m_src.n, m_src.s);\
    int m_var = atoi(buf##m_var);

/*
 * Copy m_src string to m_dst
 * Only if destination space is sufficient
 *
 * See struct snb_s
 */
#define snb_cpy_ds(m_dst, m_src)\
    m_dst.n = 0;\
    if(sizeof(m_dst.s) >= m_src.n) {\
        m_dst.n = m_src.n;\
        memcpy(m_dst.s, m_src.s, m_src.n);\
    } else { abort(); }

/*
 * Compare m_dst and m_src strings
 */
#define sn_memcmp(m_dst, m_dst_n, m_src, m_src_n)\
    (m_dst_n == m_src_n && memcmp(m_dst, m_src, m_dst_n) == 0)
#define sn_cmps(m_dst, m_src)\
    sn_memcmp(m_dst.s, m_dst.n, m_src.s, m_src.n)

/*
 * Set SN number
 */
#define sn_num_set(n_num, n_src)\
    ((n_num.n == sizeof(int)) ? *(int *)(n_num.s) = n_src :\
    ((n_num.n == sizeof(short)) ? *(short *)(n_num.s) = n_src :\
    ((n_num.n == sizeof(char)) ? *(char *)(n_num.s) = n_src : 0 )))

/*
 * Get SN number
 */
#define sn_num(n_num)\
    ((n_num.n == sizeof(int)) ? *(int *)(n_num.s) :\
    ((n_num.n == sizeof(short)) ? *(short *)(n_num.s) :\
    ((n_num.n == sizeof(char)) ? *(char *)(n_num.s) : 0 )))

/*
 * Free SN
 */
#define sn_free(dst)\
    if(dst.n > 0) {\
        if(dst.s) free(dst.s);\
        dst.s = NULL;\
        dst.n = 0;\
    }

#define EQFLAG(m_dst, m_flag) ((m_dst & m_flag) == m_flag)

enum gc_error_e {
    GC_OK,
    GC_ERROR
};

struct mem_s {
    char *s;
    int n;
    int size;
};

typedef struct sn_s {
    char *s;
    int n;
    int offset;
} sn;

typedef struct snb_s {
    char s[256];
    int n;
    int offset;
} snb;

struct pair_s {
    sn cloud;
    sn device;
    sn port_local;
    sn port_remote;
};

struct gc_s;
struct proto_s;
struct gc_config_s;

void gc_config_dump(struct gc_config_s *cfg);
int gc_config_parse(struct gc_config_s *cfg, const char *path);
int packet_send(struct gc_s *gc, struct proto_s *pr);
int parse_header(sn input, char ***argv, int *argc);
void swap_memory(char *dst, int ndst);

//void memory_append(struct mem_s *dst, const char *src, const int nsrc);
int gc_fread(char **dst, const char *fname);

inline static void timestring(char *b, const int nb)
{
    char            buf[128];
    time_t          s;
    struct timespec spec;
    long long       ms;
    struct tm       ts;

    clock_gettime(CLOCK_REALTIME, &spec);
    s = spec.tv_sec;
    ms = spec.tv_nsec / 1.0e6;

    ts = *localtime(&s);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);

    snprintf(b, nb, "[%s.%03lld] ", buf, ms);
}

inline static int fd_close(int fd)
{
    if(fd > STDERR_FILENO) return close(fd);
    else                   return GC_ERROR;
}

inline static int fd_setkeepalive(int fd)
{
    int optval       = 1;
    socklen_t optlen = sizeof(optval);

    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) != 0) {
        return GC_ERROR;
    }

    return GC_OK;
}

inline static int fd_setnonblock(int fd)
{
    int nb = 1;
    return ioctl(fd, FIONBIO, &nb);
}

#endif
