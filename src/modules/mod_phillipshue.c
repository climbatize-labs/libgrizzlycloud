#include <curl/curl.h>

#include <gc.h>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    gc_gen_ev_send(userdata, ptr, nmemb);
    return nmemb;
}

static int curl_request(struct gc_gen_client_s *client,
                        sn url, sn pf, sn cr)
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if(curl) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%.*s", sn_p(url));
        curl_easy_setopt(curl, CURLOPT_URL, buffer);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, pf.n);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    pf.s);

        snprintf(buffer, sizeof(buffer), "%.*s", sn_p(cr));
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, buffer);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,     client);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s %d\n",
                    curl_easy_strerror(res), res);

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

static void client_data(struct gc_gen_client_s *client, char *buf, const int len)
{
    struct json_tokener *tok = json_tokener_new();
    struct json_object *jobj = json_tokener_parse_ex(tok, buf, len);
    json_tokener_free(tok);

    if(jobj == NULL) {
        return;
    }

#define BND(m_dst, m_name, m_src)\
    json_object_object_get_ex(jobj, m_name, &m_src);\
    sn_setr(m_dst,\
            (char *)json_object_get_string(m_src),\
            json_object_get_string_len(m_src));

    sn snurl, snpf, sncr;
    struct json_object *url, *postfields, *customrequest;
    BND(snurl, "url",           url)
    BND(snpf,  "postfield",     postfields)
    BND(sncr,  "customrequest", customrequest)

    if(snurl.n > 0 && snpf.n > 0) {
        curl_request(client, snurl, snpf, sncr);
    }

    json_object_put(jobj);
}

static int start(struct gc_s *gc, struct gc_module_s *module)
{
    hm_log(LOG_TRACE, &gc->log, "Starting module [%s]", module->name);
    module->server = hm_palloc(gc->pool, sizeof(*(module->server)));
    if(!module->server) return GC_ERROR;

    memset(module->server, 0, sizeof(*(module->server)));

    module->server->loop = gc->loop;
    module->server->log  = &gc->log;
    module->server->pool = gc->pool;
    module->server->callback.data = client_data;
    module->server->host = "0.0.0.0";

    sn_itoa(port_local, module->port, 32);
    sn_to_char(port, port_local, 32);
    module->server->port = port;

    int ret;
    ret = async_server(module->server, gc);
    if(ret != GC_OK) {
        hm_pfree(gc->pool, module->server);
        module->server = NULL;
        return ret;
    }

    return GC_OK;
}

static int status()
{
}

static int stop(struct gc_s *gc, struct gc_module_s *module)
{
    if(module && module->server) {
        hm_log(LOG_TRACE, &gc->log, "Stopping module [%s]", module->name);
        async_server_shutdown(module->server);
        module->server = NULL;
    }
}

struct gc_module_s module_phillipshue = {
    .id      = MOD_PHILLIPSHUE,
    .name    = "mod_phillipshue",
    .start   = start,
    .status  = status,
    .stop    = stop,

    .port    = MOD_PHILLIPSHUE_PORT,
    .server  = NULL,
};
