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
#ifndef GC_API_H_
#define GC_API_H_

#define GC_DEFAULT_PORT    17040
#define GC_MAX_TUNNELS     32
#define GC_MAX_ALLOW_PORTS 32

enum gc_state_e {
    GC_CONNECTED = 0,
    GC_HANDSHAKE_SUCCESS,
    GC_DISCONNECTED
};

enum gc_cfg_type_e {
    GC_TYPE_HYBRID,
    GC_TYPE_SERVER,
    GC_TYPE_CLIENT
};

struct config_tunnel_s {
    sn cloud;
    sn device;
    int port;
    int port_local;
};

struct gc_config_s {
    sn username;
    sn password;
    sn device;

    int ntunnels;
    struct config_tunnel_s tunnels[GC_MAX_TUNNELS];

    int nallowed;
    int allowed[GC_MAX_ALLOW_PORTS];

    enum gc_cfg_type_e type;

    char file[64];

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

struct gc_init_s {
    struct ev_loop *loop;

    int port;

    const char *cfgfile;

    void (*state_changed)(struct gc_s *gc, enum gc_state_e state);

    // callbacks
    void (*callback_login)(struct gc_s *gc, sn error);
    void (*callback_device_pair)(struct gc_s *gc, struct gc_device_pair_s *pair);
};

struct gc_s {
    struct ev_loop   *loop;
    struct hm_pool_s *pool;
    struct hm_log_s  log;

    struct ev_timer connect_timer;
    struct ev_signal signal;

    //void (*message_to_set_reply)(struct gc_s *gc, struct proto_s *p);
    void (*message_from)(struct gc_s *gc, char *device, int ndevice, char *msg, int nmsg, char *type, int ntype);
    void (*cloud_online)(struct gc_s *gc, char **clients, int nclients);
    void (*announce)(struct gc_s *gc, char *device, int ndevice);
    void (*offline)(struct gc_s *gc, char *device, int ndevice);
    void (*state_changed)(struct gc_s *gc, enum gc_state_e state);

    // callbacks
    void (*callback_login)(struct gc_s *gc, sn error);
    void (*callback_device_pair)(struct gc_s *gc, struct gc_device_pair_s *pair);

    snb hostname;
    int port;

    struct client_ssl_s client;

    struct gc_config_s config;

    struct {
        sn buf;
    } net;
};

struct gc_s *gc_init(struct gc_init_s *init);
int gc_deinit(struct gc_s *gc);
int gc_reinit_size(struct gc_s *gc, int size);

void gc_force_stop();

extern struct ev_loop *loop;
extern struct hm_log_s gclog;
extern struct hm_pool_s pool;

int  gc_config_init(struct gc_config_s *cfg, const char *filename);
void gc_config_free(struct gc_config_s *cfg);
int  gc_config_required(struct gc_config_s *cfg);

extern int gc_sigterm;

#endif
