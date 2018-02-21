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
        hm_log(LOG_DEBUG, gc->log, "Parsing failed");
        return GC_ERROR;
    }

    if(net_send(gc, dst.s, dst.n) != GC_OK) {
        hm_log(LOG_DEBUG, gc->log, "Packet of size %d couldn't be sent", dst.n);
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

/*

int config_parse(struct config_s *cfg, const char *path)
{
    char *content;
    int n;

    n = gc_fread(&content, path);
    if(n <= 1) {
        return -1;
    }

    struct json_tokener *tok = json_tokener_new();
    struct json_object *jobj = json_tokener_parse_ex(tok, content, n);
    json_tokener_free(tok);

    if(jobj == NULL) {
        return -1;
    }

#define PRS(m_v, m_type)\
    struct json_object *m_v;\
    json_object_object_get_ex(jobj, #m_v, &m_v);\
    enum json_type type##m_v;\
    type##m_v = json_object_get_type(m_v);\
    if(type##m_v != m_type) {\
        return -1;\
    }

    PRS(hostname, json_type_string);
    cfg->hostname = json_object_get_string(hostname);

    PRS(port, json_type_int);
    cfg->port = json_object_get_int(port);

    PRS(username, json_type_string);
    cfg->username = json_object_get_string(username);

    PRS(password, json_type_string);
    cfg->password = json_object_get_string(password);

    PRS(device, json_type_string);
    cfg->device = json_object_get_string(device);

    cfg->jobj = jobj;
    cfg->content = content;

    return 0;
}

void config_clean(struct config_s *cfg)
{
    free(cfg->content);
    json_object_put(cfg->jobj);
}

int gc_fread(char **dst, const char *fname)
{
    FILE *pFile;
    int lSize;
    char *buffer;
    int result;

    pFile = fopen(fname, "rb");
    if(pFile == NULL) {
        return -1;
    }

    fseek(pFile , 0 , SEEK_END);
    lSize = ftell(pFile);
    rewind(pFile);

    if(lSize > MAX_FILE_SIZE) {
        fclose(pFile);
        return -1;
    }

    buffer = malloc(sizeof(char) * lSize);
    if(buffer == NULL) {
        fclose(pFile);
        return -1;
    }

    result = fread(buffer, sizeof(char), lSize, pFile);
    if(result != lSize) {
        fclose(pFile);
        free(buffer);
        return -1;
    }

    *dst = buffer;

    fclose (pFile);
    return result;
}

*/
