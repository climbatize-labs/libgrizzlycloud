#ifndef TUNNEL_H_
#define TUNNEL_H_

struct tunnel_s {
    snb cloud;
    snb pid;
    snb device;
    snb port_local;
    snb port_remote;
    snb type;

    int active;

    struct conn_server_s *server;

    struct tunnel_s *next;
};

int tunnel_add(struct gc_s *gc, struct gc_device_pair_s *pair, sn type);
int tunnel_response(struct gc_s *gc, struct proto_s *p, char **argv, int argc);
void tunnel_stop(sn pid);
void tunnel_force_stop_all();

#endif
