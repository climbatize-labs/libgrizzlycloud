#ifndef GCAPI_H_
#define GCAPI_H_

enum gc_state_e {
    GC_CONNECTED = 0,
    GC_HANDSHAKE_SUCCESS,
    GC_DISCONNECTED
};

struct config_tunnel_s {
    sn cloud;
    sn device;
    int port;
    int port_local;
};

#define MAX_TUNNELS     32
#define MAX_ALLOW_PORTS 32

struct gc_config_s {
    sn username;
    sn password;
    sn device;

    int ntunnels;
    struct config_tunnel_s tunnels[MAX_TUNNELS];

    int nallowed;
    int allowed[MAX_ALLOW_PORTS];

    struct hm_log_s *log;
    struct json_object *jobj;
    char *content;
};

struct gc_device_pair_s {
    sn cloud;
    sn pid;
    sn device;
    sn port_local;
    sn port_remote;
    sn type;
};

struct gc_s {
    struct ev_loop   *loop;
    struct hm_pool_s *pool;
    struct hm_log_s  *log;

    //void (*message_to_set_reply)(struct gc_s *gc, struct proto_s *p);
    void (*message_from)(struct gc_s *gc, char *device, int ndevice, char *msg, int nmsg, char *type, int ntype);
    void (*cloud_online)(struct gc_s *gc, char **clients, int nclients);
    void (*announce)(struct gc_s *gc, char *device, int ndevice);
    void (*offline)(struct gc_s *gc, char *device, int ndevice);
    void (*state_changed)(struct gc_s *gc, enum gc_state_e state);

    // callbacks
    void (*callback_login)(struct gc_s *gc, sn error);
    void (*callback_device_pair)(struct gc_s *gc, struct gc_device_pair_s *pair);

    const char *hostname;
    int port;
    const char *username;
    const char *password;
    const char *device;

    struct client_ssl_s client;

    struct gc_config_s config;

    struct {
        sn buf;
    } net;
};

int gc_send(sn **dst, int ndst, sn *msg, sn *type);
int gc_online(struct gc_s *gc);
int gc_init(struct ev_loop *loop, struct gc_s *gc);
int gc_deinit(struct gc_s *gc);
int gc_reinit_size(struct gc_s *gc, int size);

void gc_sigh_terminate(int __attribute__ ((unused)) signo, ev_io *user_io);

extern struct ev_loop *loop;
extern struct hm_log_s gclog;
extern struct hm_pool_s pool;

extern struct gc_s *gc;

void gc_signals(struct gc_s *gc);

int gc_config_init(struct gc_config_s *cfg, const char *filename);
void gc_config_free(struct gc_config_s *cfg);

#endif
