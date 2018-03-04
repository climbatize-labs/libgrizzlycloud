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
#include <gc.h>

static struct gc_endpoint_s *endpoints = NULL;

static void endpoint_stop_client(struct conn_client_s *c)
{
    struct gc_endpoint_s *prev = NULL;
    struct gc_endpoint_s *ent;

    assert(c);

    for(ent = endpoints; ent != NULL; prev = ent, ent = ent->next) {
        if(ent->client == c) {

            if(prev) prev->next = ent->next;
            else endpoints = ent->next;

            hm_log(LOG_TRACE, c->base.log, "Removed endpoint on fd [%.*s]",
                                           sn_p(ent->remote_fd));
            free(ent);

            break;
        }
    }
}

static int port_allowed(struct gc_s *gc, sn backend_port)
{
    int i;
    sn_atoi(port, backend_port, 8);

    for(i = 0; i < gc->config.nallowed; i++) {
        if(gc->config.allowed[i] == port) {
            return GC_OK;
        }
    }

    return GC_ERROR;
}

static void endpoint_recv(struct conn_client_s *client, char *buf, int len)
{
    struct gc_endpoint_s *ent;

    assert(client);

    for(ent = endpoints; ent != NULL; ent = ent->next) {
        if(ent->client == client) {

            // Message header
            char header[64];
            snprintf(header, sizeof(header), "tunnel_response/%.*s/%.*s",
                                             sn_p(ent->backend_port),
                                             sn_p(ent->remote_fd));
            sn_initr(snheader, header, strlen(header));

            sn_initr(tmp, "device", 6);

            struct proto_s pr = { .type = MESSAGE_TO };
            sn_set(pr.u.message_to.to,      tmp);
            sn_set(pr.u.message_to.address, ent->pid);
            sn_set(pr.u.message_to.tp,      snheader);
            sn_setr(pr.u.message_to.body,   buf, len);

            hm_log(LOG_TRACE, client->base.log, "Sending header [%.*s] and payload of %d bytes to upstream",
                                                sn_p(snheader), len);

            assert(client->base.gc);
            gc_packet_send(client->base.gc, &pr);

            return;
        }
    }

    // We should never receive data from client
    // that doesn't exist in list of endpoints
    abort();
}

static void endpoint_error(struct conn_client_s *c, enum clerr_e error)
{
    hm_log(LOG_TRACE, c->base.log, "Client error %d on fd %d, endpoint %p",
                                   error, c->base.fd, c);
    endpoint_stop_client(c);
    async_client_shutdown(c);
}

static int endpoint_add(sn key, sn remote_fd, sn backend_port,
                        sn remote_port, sn pid, struct gc_endpoint_s **ep,
                        struct gc_s *gc)
{
    struct gc_endpoint_s *ent;
    ent = malloc(sizeof(*ent));
    if(!ent) return GC_ERROR;

    snb_cpy_ds(ent->key,          key);
    snb_cpy_ds(ent->remote_fd,    remote_fd);
    snb_cpy_ds(ent->backend_port, backend_port);
    snb_cpy_ds(ent->pid,          pid);

    *ep = ent;

    struct conn_client_s *client = malloc(sizeof(*client));
    if(!client) return GC_ERROR;

    memset(client, 0, sizeof(*client));

    ent->client = client;

    // Link new endpoint
    ent->next = endpoints;
    endpoints = ent;

    sn_atoi(bp, backend_port, 32);

    client->base.loop = gc->loop;
    client->base.log  = &gc->log;
    client->base.pool = gc->pool;

    sn_initz(ip, "0.0.0.0");
    snb_cpy_ds(client->base.net.ip, ip);

    client->base.net.port  = bp;
    client->callback_data  = endpoint_recv;
    client->callback_error = endpoint_error;

    client->base.gc = gc;

    int ret;
    ret = async_client(client);
    if(ret != GC_OK) return GC_ERROR;

    hm_log(LOG_TRACE, client->base.log, "Endpoint added [remote_fd:backend_port:remote_port] [%.*s:%.*s:%.*s]",
                                        sn_p(remote_fd), sn_p(backend_port), sn_p(remote_port));

    return GC_OK;
}

static struct gc_endpoint_s *endpoint_find(sn key)
{
    struct gc_endpoint_s *ent;
    for(ent = endpoints; ent != NULL; ent = ent->next) {
        if(sn_cmps(ent->key, key)) {
            return ent;
        }
    }

    return NULL;
}

void gc_endpoints_stop_all()
{
    struct gc_endpoint_s *ent, *del;

    for(ent = endpoints; ent != NULL; ) {
        if(ent->client) {
            async_client_shutdown(ent->client);
        }
        del = ent;
        ent = ent->next;
        free(del);
    }

    endpoints = NULL;
}

int gc_endpoint_request(struct gc_s *gc, struct proto_s *p, char **argv, int argc)
{
    if(argc != 4) {
        return GC_ERROR;
    }

    sn_initr(backend_port, argv[1], strlen(argv[1]));

    if(port_allowed(gc, backend_port) != GC_OK) {
        char header[64];
        snprintf(header, sizeof(header), "tunnel_denied/%.*s",
                                         sn_p(backend_port));
        sn_initr(snheader, header, strlen(header));

        struct proto_s pr = { .type = MESSAGE_TO };
        sn_set(pr.u.message_to.to,      p->u.message_from.from_device);
        sn_set(pr.u.message_to.address, p->u.message_from.from_address);
        sn_set(pr.u.message_to.tp,      snheader);
        sn_setr(pr.u.message_to.body,   "NULL", 4);

        gc_packet_send(gc, &pr);

        return GC_OK;
    }

    sn_initr(fd, argv[2], strlen(argv[2]));

    sn_bytes_new(key,    p->u.message_from.from_address.n + fd.n);
    sn_bytes_append(key, p->u.message_from.from_address);
    sn_bytes_append(key, fd);

    struct gc_endpoint_s *ep = endpoint_find(key);

    if(!ep) {
        sn_initr(remote_port,  argv[3], strlen(argv[3]));
        sn_init(pid, p->u.message_from.from_address);

        int ret;
        ret = endpoint_add(key, fd, backend_port, remote_port,
                           pid, &ep, gc);
        if(ret != GC_OK) return ret;

        hm_log(LOG_TRACE, &gc->log, "Adding endpoint");
    } else {
        hm_log(LOG_TRACE, &gc->log, "Reusing endpoint");
    }

    hm_log(LOG_TRACE, &gc->log, "Receiving header [%s/%s/%s/%s] and payload of %d bytes",
                                argv[0], argv[1], argv[2], argv[3], p->u.message_from.body.n);

    gc_gen_ev_send(ep->client,
                   p->u.message_from.body.s,
                   p->u.message_from.body.n);

    sn_bytes_delete(key);

    return GC_OK;
}

void gc_endpoint_stop(struct hm_log_s *log, sn address, sn cloud, sn device)
{
    struct gc_endpoint_s *prev = NULL;
    struct gc_endpoint_s *ent;

    for(ent = endpoints; ent != NULL; prev = ent, ent = ent->next) {
        if(sn_cmps(ent->pid, address)) {

            async_client_shutdown(ent->client);

            if(prev) prev->next = ent->next;
            else endpoints = ent->next;

            hm_log(LOG_TRACE, log, "Removed endpoint [cloud:device:remote_fd:backend_port]\
                                   [%.*s:%.*s:%.*s:%.*s]",
                                   sn_p(cloud), sn_p(device),
                                   sn_p(ent->remote_fd),
                                   sn_p(ent->backend_port));
            free(ent);

            break;
        }
    }
}
