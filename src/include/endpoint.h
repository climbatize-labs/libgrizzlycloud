#ifndef ENDPOINT_H_
#define ENDPOINT_H_

struct endpoint_s {
    snb key;
    snb remote_fd;
    snb backend_port;
    snb pid;

    struct conn_client_s *client;

    struct endpoint_s *next;
};

int endpoint_request(struct gc_s *gc, struct proto_s *p, char **argv, int argc);
void endpoint_stop(struct hm_log_s *log, sn address, sn cloud, sn device);
void endpoints_force_stop_all();

#endif
