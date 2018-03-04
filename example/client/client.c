#include <gc.h>

static void callback_login(struct gc_s *gc, sn error)
{
    hm_log(LOG_DEBUG, &gc->log, "Login error: [%.*s]", sn_p(error));
}

static void client_login(struct gc_s *gc)
{
    struct proto_s as = { .type = ACCOUNT_LOGIN };
    sn_set(as.u.account_login.email,    gc->config.username);
    sn_set(as.u.account_login.password, gc->config.password);
    sn_set(as.u.account_login.devname,  gc->config.device);

    int ret;
    ret = gc_packet_send(gc, &as);
    if(ret != GC_OK) CALLBACK_ERROR(&gc->log, "client_login");
}

static void callback_state_changed(struct gc_s *gc, enum gc_state_e state)
{
    switch(state) {
        case GC_HANDSHAKE_SUCCESS: {
            hm_log(LOG_TRACE, &gc->log, "Connected to upstream");
            client_login(gc);
        }
        break;

        default: {
            printf("Upstream changed: %d\n", state);
        }
        break;
    }
}

int main(int argc, char **argv)
{
    struct gc_init_s gci;
    struct gc_s *gc;

    if(argc != 2) {
        exit(1);
    }

    memset(&gci, 0, sizeof(gci));

    gci.loop                   = ev_default_loop(0);
    gci.cfgfile                = argv[1];
    gci.callback.state_changed = callback_state_changed;
    gci.callback.login         = callback_login;

    gc = gc_init(&gci);
    if(gc == NULL) {
        return 1;
    }

    ev_run(gci.loop, 0);
    gc_deinit(gc);

    return 0;
}
