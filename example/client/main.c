#include <gc.h>

static void callback_pair(struct gc_s *gc, struct gc_device_pair_s *pair)
{
    printf(">%.*s\n", pair->type.n, pair->type.s);
    printf("\t>[%.*s]\n", pair->device.n, pair->device.s);

    uint ret;
    ret = tunnel_add(gc, pair, pair->type);
    if(ret != GC_OK) {
        // error
    }
}
/*
static void client_pair(struct gc_s *gc, struct pair_s *p)
{
    struct proto_s pr = { .type = DEVICE_PAIR };
    CMDSARGVS(pr.u.device_pair.cloud,       p->cloud);
    CMDSARGVS(pr.u.device_pair.device,      p->device);
    CMDSARGVS(pr.u.device_pair.local_port,  p->port_local);
    CMDSARGVS(pr.u.device_pair.remote_port, p->port_remote);

    packet_send(gc, &pr);
}
*/

static void callback_login(struct gc_s *gc, sn error)
{
    sn_initz(ok, "ok");

    printf("account login error [%.*s]\n", sn_p(error));

    /*
    if(sn_cmps(ok, error) == 0) {
        struct pair_s p;
        sn_setz(p.cloud,       "hi2");
        sn_setz(p.device,      "DevName2");
        sn_setz(p.port_local,  "1234");
        sn_setz(p.port_remote, "22");
        client_pair(gc, &p);
    }
    */
}

static void client_login(struct gc_s *gc)
{
    sn_initz(email,  "hi2");
    sn_initz(pass,   "hi2");
    sn_initz(device, "DevName2");

    struct proto_s as = { .type = ACCOUNT_LOGIN };
    sn_set(as.u.account_login.email,    email);
    sn_set(as.u.account_login.password, pass);
    sn_set(as.u.account_login.devname,  device);

    /*
    sn dst;
    if(serialize(&dst, &as) != 0) {
        printf("serialize failed\n");
        return;
    }

    if(send_packet(gc, dst.s, dst.n) != 0) {
        printf("send packe failed\n");
        return;
    }
    */
    packet_send(gc, &as);
}

static void upstream_state_changed(struct gc_s *gc, enum gc_state_e state)
{
    printf("state changed %d\n", state);
    switch(state) {
        case GC_HANDSHAKE_SUCCESS: {
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
    struct gc_s gc;

    memset(&gc, 0, sizeof(gc));

    hm_log_open(&gc.log, NULL, LOG_TRACE);

    if(argc != 2) {
        hm_log(LOG_CRIT, &gc.log, "Please specify config file");
        exit(1);
    }

    gc.config.log = &gc.log;
    assert(gc.config.log);
    if(gc_config_init(&gc.config, argv[1]) != GC_OK) {
        exit(1);
    }

    gc.loop = ev_default_loop(0);

    gc_signals(&gc);

    gc.state_changed  = upstream_state_changed;

    gc.callback_login        = callback_login;
    gc.callback_device_pair  = callback_pair;

    gc.port = 17041;
    gc.hostname = "localhost";
    gc.username = "hi1";
    gc.password = "hi1";
    gc.device   = "Dev";

    gc_init(gc.loop, &gc);

    ev_run(gc.loop, 0);

    gc_deinit(&gc);

    return 0;
}
