#include <gc.h>

static void ev_send(struct ringbuffer_s *rb,
                           struct ev_loop *loop,
                           struct ev_io *write,
                           char *buf, int len)
{
    ringbuffer_send_append(rb, buf, len);
    ev_io_start(loop, write);
}

static void memory_append(struct mem_s *dst, const char *src, const int nsrc)
{
    assert(dst->size >= nsrc + dst->n);
    memcpy(dst->s + dst->n, src, nsrc);
    dst->n += nsrc;
}

static int net_send(struct gc_s *gc, const char *json, const int njson)
{
    int len = njson;
    struct mem_s m = {
        .s = malloc(njson + 4),
        .n = 0,
        .size = njson + 4};
    int tmplen = len;

    swap_memory((void *)&len, sizeof(len));

    memory_append(&m, (void *)&len, sizeof(len));
    memory_append(&m, json, tmplen);

    struct client_ssl_s *c = &gc->client;
    gc_ev_send(c, m.s, m.n);

    free(m.s);

    return GC_OK;
}

void swap_memory(char *dst, int ndst)
{
    int i, j;

    for(i = 0, j = ndst - 1; (i < (ndst / 2) && j > 0); i++, j--) {
        dst[j] ^= dst[i];
        dst[i] ^= dst[j];
        dst[j] ^= dst[i];
    }
}

void gc_ev_send(struct client_ssl_s *client, char *buf, const int len)
{
    ev_send(&client->base.rb, client->base.loop,
            &client->base.write, buf, len);
}

void gen_ev_send(struct conn_client_s *client, char *buf, const int len)
{
    ev_send(&client->base.rb, client->base.loop,
            &client->base.write, buf, len);
}

int packet_send(struct gc_s *gc, struct proto_s *pr)
{
    sn dst;
    if(serialize(&dst, pr) != GC_OK) {
        hm_log(LOG_DEBUG, &gc->log, "Packet serialization failed");
        return GC_ERROR;
    }

    if(net_send(gc, dst.s, dst.n) != GC_OK) {
        hm_log(LOG_DEBUG, &gc->log, "Packet of size %d couldn't be sent", dst.n);
        return GC_ERROR;
    }

    sn_free(dst);

    return GC_OK;
}

int parse_header(sn input, char ***argv, int *argc)
{
    char *start, *tmp;

#define ASET\
    *argv = realloc(*argv, (size_t)((++(*argc)) * sizeof(void *)));\
    if(!argv) return GC_ERROR;\
    (*argv)[*argc - 1] = start;

    for(start = tmp = input.s, *argc = 0, *argv = NULL;
        tmp < (input.s + input.n);
        tmp++) {
        if(*tmp == '/') {
            ASET
            start = tmp + 1;
            *tmp = '\0'; // replace / with zero to terminate str
        }
    }

    ASET

    return GC_OK;
}

int gc_config_parse(struct gc_config_s *cfg, const char *path)
{
    char *content;
    int n;

    assert(cfg);

    n = gc_fread(&content, path);
    if(n <= 1) {
        return GC_ERROR;
    }

    // Trim long paths up to sizeof(cfg->file) bytes
    snprintf(cfg->file, sizeof(cfg->file), "%s", path);

    struct json_tokener *tok = json_tokener_new();
    struct json_object *jobj = json_tokener_parse_ex(tok, content, n);
    json_tokener_free(tok);

    if(jobj == NULL) {
        return GC_ERROR;
    }

#define PRS(m_v, m_type)\
    struct json_object *m_v;\
    json_object_object_get_ex(jobj, #m_v, &m_v);\
    enum json_type type##m_v;\
    type##m_v = json_object_get_type(m_v);\
    if(type##m_v != m_type) {\
        return GC_ERROR;\
    }

#define VAL_STR(m_dst, m_src)\
    PRS(m_src, json_type_string);\
    sn_setr(m_dst,\
            (char *)json_object_get_string(m_src),\
            json_object_get_string_len(m_src));

    VAL_STR(cfg->username, user)
    VAL_STR(cfg->password, password)

    struct json_object *device;
    json_object_object_get_ex(jobj, "device", &device);
    if(json_object_get_type(device) == json_type_string) {
        sn_setr(cfg->device,
            (char *)json_object_get_string(device),
            json_object_get_string_len(device));
    }

    struct json_object *allow;
    json_object_object_get_ex(jobj, "allow", &allow);
    if(json_object_get_type(allow) == json_type_array) {
        array_list *allow_array = json_object_get_array(allow);
        int i;
        for(i = 0; i < array_list_length(allow_array); i++) {
            struct json_object *port = array_list_get_idx(allow_array, i);
            cfg->allowed[i] = json_object_get_int(port);
            cfg->nallowed++;
        }
    }

    struct json_object *tunnels;
    json_object_object_get_ex(jobj, "tunnels", &tunnels);
    if(json_object_get_type(tunnels) == json_type_array) {
        array_list *tunnels_array = json_object_get_array(tunnels);

        int i;
        for(i = 0; i < array_list_length(tunnels_array); i++) {
            struct json_object *tunnel = array_list_get_idx(tunnels_array, i);
            struct json_object *t_cloud, *t_device, *t_port, *t_port_local;

#define TUN(m_dst, m_name, m_src)\
        json_object_object_get_ex(tunnel, m_name, &m_src);\
        sn_setr(m_dst,\
                (char *)json_object_get_string(m_src),\
                json_object_get_string_len(m_src));

#define TUN_INT(m_dst, m_name, m_src)\
        json_object_object_get_ex(tunnel, m_name, &m_src);\
        m_dst = json_object_get_int(m_src);

            TUN(cfg->tunnels[i].cloud,      "cloud",     t_cloud)
            TUN(cfg->tunnels[i].device,     "device",    t_device)

            TUN_INT(cfg->tunnels[i].port,       "port",      t_port)
            TUN_INT(cfg->tunnels[i].port_local, "portLocal", t_port_local)

            cfg->ntunnels++;
        }
    }

    cfg->jobj = jobj;
    cfg->content = content;

    return GC_OK;
}

void gc_config_dump(struct gc_config_s *cfg)
{
    int i;

    hm_log(LOG_DEBUG, cfg->log, "Using config: %s", cfg->file);
    hm_log(LOG_DEBUG, cfg->log, "Username: [%.*s]", sn_p(cfg->username));
    hm_log(LOG_DEBUG, cfg->log, "Password: [%s]", cfg->password.n > 0 ? "Set" : "Not Set");
    hm_log(LOG_DEBUG, cfg->log, "Device: [%.*s]", sn_p(cfg->device));

    hm_log(LOG_DEBUG, cfg->log, "Allowed ports total: [%d]", cfg->nallowed);

    for(i = 0; i < cfg->nallowed; i++) {
        hm_log(LOG_DEBUG, cfg->log, "Allowed port: [%d]", cfg->allowed[i]);
    }

    hm_log(LOG_DEBUG, cfg->log, "Allowed tunnels total: [%d]", cfg->ntunnels);

    for(i = 0; i < cfg->ntunnels; i++) {
        hm_log(LOG_DEBUG, cfg->log, "Tunnel %d: Cloud: [%.*s] Device: [%.*s] Port: [%d] Local port: [%d]",
                                    i,
                                    sn_p(cfg->tunnels[i].cloud),
                                    sn_p(cfg->tunnels[i].device),
                                    cfg->tunnels[i].port,
                                    cfg->tunnels[i].port_local);
    }

}

void config_clean(struct gc_config_s *cfg)
{
    free(cfg->content);
    json_object_put(cfg->jobj);
}

int gc_fread(char **dst, const char *fname)
{
    FILE *pfile;
    int lsize;
    char *buffer;
    int result;

    pfile = fopen(fname, "rb");
    if(pfile == NULL) {
        return -1;
    }

    fseek(pfile , 0 , SEEK_END);
    lsize = ftell(pfile);
    rewind(pfile);

    if(lsize > MAX_FILE_SIZE) {
        fclose(pfile);
        return -1;
    }

    buffer = malloc(sizeof(char) * lsize);
    if(buffer == NULL) {
        fclose(pfile);
        return -1;
    }

    result = fread(buffer, sizeof(char), lsize, pfile);
    if(result != lsize) {
        fclose(pfile);
        free(buffer);
        return -1;
    }

    *dst = buffer;

    fclose(pfile);
    return result;
}
