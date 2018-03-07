/*
 *
 * GrizzlyCloud library - simplified VPN alternative for IoT
 * Copyright (C) 2017 - 2018 Filip Pancik
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef GC_UTILS_H_
#define GC_UTILS_H_

#define GC_MAX_FILE_SIZE   (1024 * 1024)

#define COUNT(m_dst) sizeof(m_dst) / sizeof(m_dst[0])

#define CALLBACK_ERROR(m_log, m_msg)\
    hm_log(LOG_EMERG, m_log, m_msg)

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
 * String length
 */
#define sn_len(m_var) m_var.n

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
 * Convert int m_src to string m_var
 * while m_size is conversion buffer size
 */
#define sn_itoa(m_var, m_src, m_size)\
    char buf##m_var[m_size];\
    snprintf(buf##m_var, sizeof(buf##m_var), "%d", m_src);\
    sn_initz(m_var, buf##m_var);

/*
 * Zero initialize m_dst
 */
#define snb_zero(m_dst)\
    snb m_dst = { .n = 0, .offset = 0 }

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
 * Copy m_src string to m_dst
 * Only if destination space is sufficient
 *
 * See struct snb_s
 */
#define snb_cpy_d(m_dst, m_src)\
    m_dst->n = 0;\
    if(sizeof(m_dst->s) >= m_src.n) {\
        m_dst->n = m_src.n;\
        memcpy(m_dst->s, m_src.s, m_src.n);\
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

/**
 * @brief GC enum.
 *
 * Used as an internal function return value error indicator.
 */
enum gc_e {
    GC_OK,      /**< Non error return. */
    GC_ERROR    /**< Generic error. */
};

/**
 * @brief Memory region S of N characters.
 *
 * Defines memory region for later manipulation.
 */
typedef struct sn_s {
    char *s;     /**< Pointer to a memory region */
    int  n;      /**< Size of memory region */
    int  offset; /**< Offset from start of region */
} sn;

/**
 * @brief Preallocated memory region S of sizeof(S) characters.
 *
 * Defines memory region for later manipulation
 */
typedef struct snb_s {
    char s[256];  /**< Pointer to a memory region */
    int  n;       /**< Size of memory region snb#s */
    int  offset;  /**< Offset from start of region */
} snb;

struct gc_s;
struct proto_s;
struct gc_config_s;

/**
 * @brief Dump config.
 *
 * Dumps configuration structure
 * @param cfg Config structure
 * @return void
 */
void gc_config_dump(struct gc_config_s *cfg);

/**
 * @brief Parse config file.
 *
 * Read @p path and fill @p cfg.
 * @param cfg Config structure
 * @param path Path to filename
 * @return GC_OK on success, GC_ERROR on failure
 */
int gc_config_parse(struct gc_config_s *cfg, const char *path);

/**
 * @brief Send packet to upstream.
 *
 * @param gc GC structure.
 * @param pr Protocol message.
 * @return GC_OK on success, GC_ERROR on failure.
 */
int gc_packet_send(struct gc_s *gc, struct proto_s *pr);

/**
 * @brief Parse buffer by delimiter.
 *
 * @param input Buffer.
 * @param argv Pointer to array of parsed elements.
 * @param argc Number of elements in array.
 * @param delimiter Delimiter.
 * @return GC_OK on success, GC_ERROR on failure.
 */
int gc_parse_delimiter(sn input, char ***argv, int *argc, char delimiter);

/**
 * @brief Swap memory.
 *
 * @param dst Memory region pointer.
 * @param ndst Memory region size.
 * @return void.
 */
void gc_swap_memory(char *dst, int ndst);

/**
 * @brief Read file.
 *
 * Read file @p fname and put its output to @p dst.
 * @param dst Pointer to memory region filled with file content.
 * @param fname File to read.
 * @return On failure -1, otherwise, size of read bytes.
 */
int gc_fread(char **dst, const char *fname);

/**
 * @brief Create string representation of time.
 *
 * @param b Buffer to fill with characters.
 * @param nb Maximum size of buffer.
 * @return void.
 */
inline static void gc_timestring(char *b, const int nb)
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

/**
 * @brief Close file descriptor.
 *
 * @param fd File descriptor.
 * @return GC_OK on success, GC_ERROR, -1 and errno is set on failure.
 */
inline static int gc_fd_close(int fd)
{
    if(fd > STDERR_FILENO) return close(fd);
    else                   return GC_ERROR;
}

/**
 * @brief Set KEEPALIVE on file descriptor.
 *
 * @param fd File descriptor.
 * @return GC_OK on success, GC_ERROR on failure.
 */
inline static int gc_fd_setkeepalive(int fd)
{
    int optval       = 1;
    socklen_t optlen = sizeof(optval);

    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) != 0) {
        return GC_ERROR;
    }

    return GC_OK;
}

/**
 * @brief Set NONBLOCKING file descriptor.
 *
 * @param fd File descriptor.
 * @return GC_OK on success, -1 and errno is set on failure.
 */
inline static int gc_fd_setnonblock(int fd)
{
    int nb = 1;
    return ioctl(fd, FIONBIO, &nb);
}

#endif
