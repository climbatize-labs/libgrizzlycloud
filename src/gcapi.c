#include <gc.h>

static ev_timer connect_timer;

struct gc_s *gc;

int gc_message_from(struct gc_s *gc, struct proto_s *p)
{
    char **argv;
    int argc;
    int ret;

    ret = parse_header(p->u.message_from.tp, &argv, &argc);
    if(ret != GC_OK) {
        if(argv) free(argv);
        return ret;
    }

    if(argc < 1) {
        if(argv) free(argv);
        return GC_ERROR;
    }

    sn_initr(type, argv[0], strlen(argv[0]));
    sn_initz(response, "tunnel_response");
    sn_initz(request, "tunnel_request");

    if(sn_cmps(type, request)) {
        endpoint_request(gc, p, argv, argc);
    } else if(sn_cmps(type, response)) {
        tunnel_response(gc, p, argv, argc);
    }

    free(argv);

    return GC_OK;
}

static void gc_cloud_offline(struct gc_s *gc, struct proto_s *p)
{
    endpoint_stop(gc->log,
                  p->u.offline_set.address,
                  p->u.offline_set.cloud,
                  p->u.offline_set.device);

    tunnel_stop(p->u.offline_set.address);
}

static void release(struct client_ssl_s *c, enum gcerr_e error)
{
    async_client_ssl_shutdown(c);
    connect_timer.data = NULL;
    ev_timer_again(loop, &connect_timer);
    (void)error;
}

static void receive(struct gc_s *gc, const void *buffer, const int nbuffer)
{
    struct proto_s p;
    sn src = { .s = (char *)buffer + 4, .n = nbuffer - 4 };
    if(deserialize(&p, &src) != 0) {
        hm_log(LOG_ERR, gc->log, "Parsing failed");
        return;
    }

    hm_log(LOG_TRACE, gc->log, "Received packet from upstream type: %d size: %d",
                               p.type, nbuffer);

    switch(p.type) {
        case ACCOUNT_LOGIN_REPLY:
            if(gc->callback_login)
                gc->callback_login(gc, p.u.account_login_reply.error);
        break;
        case DEVICE_PAIR_REPLY: {
            sn_initz(ok, "ok");
            if(sn_cmps(ok, p.u.device_pair_reply.error)) {
                sn list = p.u.device_pair_reply.list;

// WARNING: not alligned, endian ignorant
#define READ(m_var)\
            if(i < list.n) {\
                m_var.n = *(int *)(&((list.s)[i]));\
                /* Swap memory because of server high endian */\
                swap_memory((void *)&m_var.n, sizeof(m_var.n));\
                m_var.s = &((list.s)[i + 4]);\
                i += sizeof(m_var.n) + m_var.n;\
            }

                int i;
                for(i = 0; i < list.n; i++) {
                    struct gc_device_pair_s pair;
                    sn_set(pair.cloud, p.u.device_pair_reply.cloud);
                    READ(pair.pid);
                    READ(pair.device);
                    READ(pair.port_local);
                    READ(pair.port_remote);
                    sn_set(pair.type, p.u.device_pair_reply.type);

                    if(gc->callback_device_pair)
                        gc->callback_device_pair(gc, &pair);
                }
            }
            }
        break;
        case MESSAGE_FROM: {
                gc_message_from(gc, &p);
            }
        break;
        case OFFLINE_SET: {
                gc_cloud_offline(gc, &p);
            }
        break;
        default:
            hm_log(LOG_TRACE, gc->log, "Not handling packet type: %d size: %d", p.type, nbuffer);
        break;
    }
}

static void try_connect(struct ev_loop *loop, struct ev_timer *timer, int revents)
{
    ev_timer_stop(loop, &connect_timer);

    memset(&gc->client, 0, sizeof(gc->client));

    gc->client.base.loop = loop;
    gc->client.base.log  = gc->log;

    sn_initz(hostname, (char *)gc->hostname);
    gc->client.base.net.port = gc->port;
    snb_cpy_ds(gc->client.base.net.ip, hostname);

    gc->client.recv = receive;

    gc->client.error_callback = release;
    timer->data = gc;

    async_client_ssl(gc);
    (void )revents;
}

int gc_deinit(struct gc_s *gc)
{
    if(gc->net.buf.s) free(gc->net.buf.s);

    FIPS_mode_set(0);
    ENGINE_cleanup();
    CONF_modules_unload(1);
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_remove_state(0);
    ERR_free_strings();

    ev_default_destroy();

    return 0;
}

int gc_init(struct ev_loop *l, struct gc_s *callbacks)
{
    loop = l;

    if(SSL_library_init() < 0) {
        hm_log(LOG_CRIT, gc->log, "Could not initialize OpenSSL library");
        return GC_ERROR;
    }

    SSL_load_error_strings();

    ev_init(&connect_timer, try_connect);

    connect_timer.repeat = 2.0;
    connect_timer.data = NULL;
    gc = callbacks;

    gc->net.buf.n = 0;
    gc->net.buf.s = NULL;

    ev_timer_again(loop, &connect_timer);

    return GC_OK;
}

static void gc_force_stop()
{
    ev_timer_stop(loop, &connect_timer);

    async_client_ssl_shutdown(&gc->client);
}

static void sigh_terminate(int __attribute__ ((unused)) signo)
{
    hm_log(LOG_TRACE, gc->log, "Received SIGTERM");
    gc_force_stop();
    tunnel_force_stop_all();
    endpoints_force_stop_all();
}

void gc_signals(struct gc_s *gc)
{
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = SIG_IGN;

    if(sigaction(SIGPIPE, &act, NULL) < 0) {
        hm_log(LOG_CRIT, gc->log, "Sigaction cannot be examined");
    }

    act.sa_flags = SA_NOCLDSTOP;

    act.sa_flags = 0;
    act.sa_handler = sigh_terminate;
    if(sigaction(SIGINT, &act, NULL) < 0) {
        hm_log(LOG_CRIT, gc->log, "Unable to register SIGINT signal handler: %s", strerror(errno));
        exit(1);
    }

    if(sigaction(SIGTERM, &act, NULL) < 0) {
        hm_log(LOG_CRIT, gc->log, "Unable to register SIGTERM signal handler: %s", strerror(errno));
        exit(1);
    }
}
