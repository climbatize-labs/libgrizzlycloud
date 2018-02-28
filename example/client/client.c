#include <gc.h>

#include <client.h>

static void callback_pair(struct gc_s *gc, struct gc_device_pair_s *pair)
{
    (void )gc_tunnel_add(gc, pair, pair->type);
}

static void client_pair(struct gc_s *gc, struct pair_s *p)
{
    struct proto_s pr = { .type = DEVICE_PAIR };
    sn_set(pr.u.device_pair.cloud,       p->cloud);
    sn_set(pr.u.device_pair.device,      p->device);

    sn_itoa(port,       p->port_remote, 8);
    sn_itoa(port_local, p->port_local,  8);

    sn_set(pr.u.device_pair.local_port,  port_local);
    sn_set(pr.u.device_pair.remote_port, port);

    int ret;
    ret = gc_packet_send(gc, &pr);
    if(ret != GC_OK) CALLBACK_ERROR(&gc->log, "client_pair");
}

static void callback_login(struct gc_s *gc, sn error)
{
    sn_initz(ok, "ok");
    sn_initz(ok_reg, "ok_registered");

    hm_log(LOG_DEBUG, &gc->log, "Login error: [%.*s]", sn_p(error));

    if(sn_cmps(ok, error)) {

        int i;
        for(i = 0; i < gc->config.ntunnels; i++) {
            struct pair_s p;
            sn_set(p.cloud,  gc->config.tunnels[i].cloud);
            sn_set(p.device, gc->config.tunnels[i].device);
            p.port_remote =  gc->config.tunnels[i].port;
            p.port_local =   gc->config.tunnels[i].port_local;
            client_pair(gc, &p);
        }

    } else if(!sn_cmps(ok_reg, error)) {
        gc_force_stop();
    }
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

static void upstream_state_changed(struct gc_s *gc, enum gc_state_e state)
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

    gci.loop                 = ev_default_loop(0);
    gci.cfgfile              = argv[1];
    gci.state_changed        = upstream_state_changed;
    gci.callback_login       = callback_login;
    gci.callback_device_pair = callback_pair;
    gci.port                 = GC_DEFAULT_PORT;

    sn_setz(gci.hostname, "localhost");

    gc = gc_init(&gci);
    if(gc == NULL) {
        return 1;
    }

    ev_run(gci.loop, 0);
    gc_deinit(gc);

    return 0;
}
