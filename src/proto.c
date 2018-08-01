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
// This file is generated
#include <gc.h>

#define add_uint(m_pool, m_uint)\
    { int ret = add_int_intern(m_pool, dst, m_uint);\
      if (ret != GCPROTO_OK) return ret; }

#define add_bin(m_pool, m_bin, m_nbin)\
    { int ret = add_intern(m_pool, dst, m_bin, m_nbin);\
      if (ret != GCPROTO_OK) return ret; }

void gc_proto_dump(struct proto_s *p)
{
    switch(p->type) {
    case MESSAGE_TO:
        printf("MESSAGE_TO\n");
        printf("to: %.*s\n", p->u.message_to.to.n, p->u.message_to.to.s);
        printf("address: %.*s\n", p->u.message_to.address.n, p->u.message_to.address.s);
        printf("tp: %.*s\n", p->u.message_to.tp.n, p->u.message_to.tp.s);
        printf("body: %.*s\n", p->u.message_to.body.n, p->u.message_to.body.s);
        break;
    case MESSAGE_TO_SET_REPLY:
        printf("MESSAGE_TO_SET_REPLY\n");
        printf("error: %.*s\n", p->u.message_to_set_reply.error.n, p->u.message_to_set_reply.error.s);
        break;
    case PING_SET:
        printf("PING_SET\n");
        break;
    case PONG_SET:
        printf("PONG_SET\n");
        break;
    case ACCOUNT_LIST:
        printf("ACCOUNT_LIST\n");
        break;
    case ACCOUNT_LIST_REPLY:
        printf("ACCOUNT_LIST_REPLY\n");
        printf("error: %.*s\n", p->u.account_list_reply.error.n, p->u.account_list_reply.error.s);
        printf("list: %.*s\n", p->u.account_list_reply.list.n, p->u.account_list_reply.list.s);
        break;
    case TRAFFIC_MI:
        printf("TRAFFIC_MI\n");
        break;
    case TRAFFIC_GET:
        printf("TRAFFIC_GET\n");
        break;
    case TRAFFIC_GET_REPLY:
        printf("TRAFFIC_GET_REPLY\n");
        printf("list: %.*s\n", p->u.traffic_get_reply.list.n, p->u.traffic_get_reply.list.s);
        printf("error: %.*s\n", p->u.traffic_get_reply.error.n, p->u.traffic_get_reply.error.s);
        break;
    case MESSAGE_FROM:
        printf("MESSAGE_FROM\n");
        printf("from_cloud: %.*s\n", p->u.message_from.from_cloud.n, p->u.message_from.from_cloud.s);
        printf("from_device: %.*s\n", p->u.message_from.from_device.n, p->u.message_from.from_device.s);
        printf("from_address: %.*s\n", p->u.message_from.from_address.n, p->u.message_from.from_address.s);
        printf("tp: %.*s\n", p->u.message_from.tp.n, p->u.message_from.tp.s);
        printf("body: %.*s\n", p->u.message_from.body.n, p->u.message_from.body.s);
        break;
    case DEVICE_PAIR:
        printf("DEVICE_PAIR\n");
        printf("cloud: %.*s\n", p->u.device_pair.cloud.n, p->u.device_pair.cloud.s);
        printf("device: %.*s\n", p->u.device_pair.device.n, p->u.device_pair.device.s);
        printf("local_port: %.*s\n", p->u.device_pair.local_port.n, p->u.device_pair.local_port.s);
        printf("remote_port: %.*s\n", p->u.device_pair.remote_port.n, p->u.device_pair.remote_port.s);
        break;
    case DEVICE_PAIR_REPLY:
        printf("DEVICE_PAIR_REPLY\n");
        printf("cloud: %.*s\n", p->u.device_pair_reply.cloud.n, p->u.device_pair_reply.cloud.s);
        printf("error: %.*s\n", p->u.device_pair_reply.error.n, p->u.device_pair_reply.error.s);
        printf("list: %.*s\n", p->u.device_pair_reply.list.n, p->u.device_pair_reply.list.s);
        printf("type: %.*s\n", p->u.device_pair_reply.type.n, p->u.device_pair_reply.type.s);
        break;
    case OFFLINE_SET:
        printf("OFFLINE_SET\n");
        printf("address: %.*s\n", p->u.offline_set.address.n, p->u.offline_set.address.s);
        printf("cloud: %.*s\n", p->u.offline_set.cloud.n, p->u.offline_set.cloud.s);
        printf("device: %.*s\n", p->u.offline_set.device.n, p->u.offline_set.device.s);
        break;
    case ACCOUNT_SET:
        printf("ACCOUNT_SET\n");
        printf("email: %.*s\n", p->u.account_set.email.n, p->u.account_set.email.s);
        printf("password: %.*s\n", p->u.account_set.password.n, p->u.account_set.password.s);
        break;
    case ACCOUNT_SET_REPLY:
        printf("ACCOUNT_SET_REPLY\n");
        printf("error: %.*s\n", p->u.account_set_reply.error.n, p->u.account_set_reply.error.s);
        break;
    case ACCOUNT_GET:
        printf("ACCOUNT_GET\n");
        break;
    case ACCOUNT_LOGIN:
        printf("ACCOUNT_LOGIN\n");
        printf("email: %.*s\n", p->u.account_login.email.n, p->u.account_login.email.s);
        printf("password: %.*s\n", p->u.account_login.password.n, p->u.account_login.password.s);
        printf("devname: %.*s\n", p->u.account_login.devname.n, p->u.account_login.devname.s);
        break;
    case ACCOUNT_LOGIN_REPLY:
        printf("ACCOUNT_LOGIN_REPLY\n");
        printf("error: %.*s\n", p->u.account_login_reply.error.n, p->u.account_login_reply.error.s);
        break;
    case ACCOUNT_EXISTS:
        printf("ACCOUNT_EXISTS\n");
        printf("email: %.*s\n", p->u.account_exists.email.n, p->u.account_exists.email.s);
        printf("password: %.*s\n", p->u.account_exists.password.n, p->u.account_exists.password.s);
        break;
    case ACCOUNT_EXISTS_REPLY:
        printf("ACCOUNT_EXISTS_REPLY\n");
        printf("error: %.*s\n", p->u.account_exists_reply.error.n, p->u.account_exists_reply.error.s);
        break;
    case VERSION_MISMATCH:
        printf("VERSION_MISMATCH\n");
        printf("master: %.*s\n", p->u.version_mismatch.master.n, p->u.version_mismatch.master.s);
        printf("slave: %.*s\n", p->u.version_mismatch.slave.n, p->u.version_mismatch.slave.s);
        break;

        default:
            return;
            break;
    }
}

static int add_intern(struct hm_pool_s *pool, sn *dst, const void *src, const int nsrc)
{
    assert(dst);

    int offset_0 = dst->n;
    int offset_1 = dst->n + sizeof(nsrc);

    dst->n += nsrc + sizeof(nsrc);
    dst->s  = hm_prealloc(pool, dst->s, dst->n);

    assert(dst->s);

    (void )memcpy(dst->s + offset_0, &nsrc, sizeof(nsrc));
    gc_swap_memory(dst->s + offset_0, sizeof(nsrc));
    (void )memcpy(dst->s + offset_1, src, nsrc);

    return GCPROTO_OK;
}

static int add_int_intern(struct hm_pool_s *pool, sn *dst, int v)
{
    assert(dst);

    int offset_0 = dst->n;

    dst->n += sizeof(v);
    dst->s  = hm_prealloc(pool, dst->s, dst->n);

    assert(dst->s);

    (void )memcpy(dst->s + offset_0, &v, sizeof(v));
    gc_swap_memory(dst->s + offset_0, sizeof(v));

    return GCPROTO_OK;
}

static int get_int_intern(sn *src, int *value)
{
    assert(src);

    if ((int)(src->offset + sizeof(int)) > src->n) {
        return GCPROTO_ERR;
    }

    *value = *(int *)(src->s + src->offset);
    gc_swap_memory((void*)value, sizeof(*value));
    src->offset += sizeof(*value);

    return GCPROTO_OK;
}

static int get_intern(sn *dst, sn *src)
{
    if (src->offset + (int)sizeof(int) > src->n) {
        return GCPROTO_ERR;
    }

    dst->n = *(int *)(src->s + src->offset);
    dst->s = src->s + src->offset + sizeof(int);

    gc_swap_memory((void*)&dst->n, sizeof(dst->n));

    // increment offset
    src->offset += sizeof(int) + dst->n;

    return GCPROTO_OK;
}

static int get_enum(sn *src, enum proto_e *value)
{
    return get_int_intern(src, (int*)value);
}

static int get_int(sn *src, int *value)
{
    return get_int_intern(src, (int *)value);
}

int gc_serialize(struct hm_pool_s *pool, sn *dst, struct proto_s *src)
{
    dst->s = NULL;
    dst->n = dst->offset = 0;

    add_uint(pool, GCPROTO_VERSION);
    add_uint(pool, src->type);

    switch(src->type) {
        case MESSAGE_TO:
            add_intern(pool, dst, src->u.message_to.to.s, src->u.message_to.to.n);
            add_intern(pool, dst, src->u.message_to.address.s, src->u.message_to.address.n);
            add_intern(pool, dst, src->u.message_to.tp.s, src->u.message_to.tp.n);
            add_intern(pool, dst, src->u.message_to.body.s, src->u.message_to.body.n);
            break;
        case MESSAGE_TO_SET_REPLY:
            if (
                (sn_memcmp("ok", 2, src->u.message_to_set_reply.error.s, src->u.message_to_set_reply.error.n) == 0) ||
                (sn_memcmp("ok_registered", 13, src->u.message_to_set_reply.error.s, src->u.message_to_set_reply.error.n) == 0) ||
                (sn_memcmp("login", 5, src->u.message_to_set_reply.error.s, src->u.message_to_set_reply.error.n) == 0) ||
                (sn_memcmp("enoexists", 9, src->u.message_to_set_reply.error.s, src->u.message_to_set_reply.error.n) == 0) ||
                (sn_memcmp("general_failure", 15, src->u.message_to_set_reply.error.s, src->u.message_to_set_reply.error.n) == 0)                 ) {
                    add_intern(pool, dst, src->u.message_to_set_reply.error.s, src->u.message_to_set_reply.error.n);
                } else { return -1; }
            break;
        case PING_SET:
            break;
        case PONG_SET:
            break;
        case ACCOUNT_LIST:
            break;
        case ACCOUNT_LIST_REPLY:
            if (
                (sn_memcmp("ok", 2, src->u.account_list_reply.error.s, src->u.account_list_reply.error.n) == 0) ||
                (sn_memcmp("general_failure", 15, src->u.account_list_reply.error.s, src->u.account_list_reply.error.n) == 0)                 ) {
                    add_intern(pool, dst, src->u.account_list_reply.error.s, src->u.account_list_reply.error.n);
                } else { return -1; }
            add_intern(pool, dst, src->u.account_list_reply.list.s, src->u.account_list_reply.list.n);
            break;
        case TRAFFIC_MI:
            break;
        case TRAFFIC_GET:
            break;
        case TRAFFIC_GET_REPLY:
            add_intern(pool, dst, src->u.traffic_get_reply.list.s, src->u.traffic_get_reply.list.n);
            if (
                (sn_memcmp("ok", 2, src->u.traffic_get_reply.error.s, src->u.traffic_get_reply.error.n) == 0) ||
                (sn_memcmp("ok_partial", 10, src->u.traffic_get_reply.error.s, src->u.traffic_get_reply.error.n) == 0) ||
                (sn_memcmp("login", 5, src->u.traffic_get_reply.error.s, src->u.traffic_get_reply.error.n) == 0) ||
                (sn_memcmp("general_failure", 15, src->u.traffic_get_reply.error.s, src->u.traffic_get_reply.error.n) == 0) ||
                (sn_memcmp("denied", 6, src->u.traffic_get_reply.error.s, src->u.traffic_get_reply.error.n) == 0) ||
                (sn_memcmp("empty", 5, src->u.traffic_get_reply.error.s, src->u.traffic_get_reply.error.n) == 0)                 ) {
                    add_intern(pool, dst, src->u.traffic_get_reply.error.s, src->u.traffic_get_reply.error.n);
                } else { return -1; }
            break;
        case MESSAGE_FROM:
            add_intern(pool, dst, src->u.message_from.from_cloud.s, src->u.message_from.from_cloud.n);
            add_intern(pool, dst, src->u.message_from.from_device.s, src->u.message_from.from_device.n);
            add_intern(pool, dst, src->u.message_from.from_address.s, src->u.message_from.from_address.n);
            add_intern(pool, dst, src->u.message_from.tp.s, src->u.message_from.tp.n);
            add_intern(pool, dst, src->u.message_from.body.s, src->u.message_from.body.n);
            break;
        case DEVICE_PAIR:
            add_intern(pool, dst, src->u.device_pair.cloud.s, src->u.device_pair.cloud.n);
            add_intern(pool, dst, src->u.device_pair.device.s, src->u.device_pair.device.n);
            add_intern(pool, dst, src->u.device_pair.local_port.s, src->u.device_pair.local_port.n);
            add_intern(pool, dst, src->u.device_pair.remote_port.s, src->u.device_pair.remote_port.n);
            break;
        case DEVICE_PAIR_REPLY:
            add_intern(pool, dst, src->u.device_pair_reply.cloud.s, src->u.device_pair_reply.cloud.n);
            if (
                (sn_memcmp("ok", 2, src->u.device_pair_reply.error.s, src->u.device_pair_reply.error.n) == 0) ||
                (sn_memcmp("ok_registered", 13, src->u.device_pair_reply.error.s, src->u.device_pair_reply.error.n) == 0) ||
                (sn_memcmp("login", 5, src->u.device_pair_reply.error.s, src->u.device_pair_reply.error.n) == 0) ||
                (sn_memcmp("general_failure", 15, src->u.device_pair_reply.error.s, src->u.device_pair_reply.error.n) == 0)                 ) {
                    add_intern(pool, dst, src->u.device_pair_reply.error.s, src->u.device_pair_reply.error.n);
                } else { return -1; }
            add_intern(pool, dst, src->u.device_pair_reply.list.s, src->u.device_pair_reply.list.n);
            add_intern(pool, dst, src->u.device_pair_reply.type.s, src->u.device_pair_reply.type.n);
            break;
        case OFFLINE_SET:
            add_intern(pool, dst, src->u.offline_set.address.s, src->u.offline_set.address.n);
            add_intern(pool, dst, src->u.offline_set.cloud.s, src->u.offline_set.cloud.n);
            add_intern(pool, dst, src->u.offline_set.device.s, src->u.offline_set.device.n);
            break;
        case ACCOUNT_SET:
            add_intern(pool, dst, src->u.account_set.email.s, src->u.account_set.email.n);
            add_intern(pool, dst, src->u.account_set.password.s, src->u.account_set.password.n);
            break;
        case ACCOUNT_SET_REPLY:
            if (
                (sn_memcmp("ok", 2, src->u.account_set_reply.error.s, src->u.account_set_reply.error.n) == 0) ||
                (sn_memcmp("denied", 6, src->u.account_set_reply.error.s, src->u.account_set_reply.error.n) == 0) ||
                (sn_memcmp("already_exists", 14, src->u.account_set_reply.error.s, src->u.account_set_reply.error.n) == 0) ||
                (sn_memcmp("general_failure", 15, src->u.account_set_reply.error.s, src->u.account_set_reply.error.n) == 0)                 ) {
                    add_intern(pool, dst, src->u.account_set_reply.error.s, src->u.account_set_reply.error.n);
                } else { return -1; }
            break;
        case ACCOUNT_GET:
            break;
        case ACCOUNT_LOGIN:
            add_intern(pool, dst, src->u.account_login.email.s, src->u.account_login.email.n);
            add_intern(pool, dst, src->u.account_login.password.s, src->u.account_login.password.n);
            add_intern(pool, dst, src->u.account_login.devname.s, src->u.account_login.devname.n);
            break;
        case ACCOUNT_LOGIN_REPLY:
            if (
                (sn_memcmp("ok", 2, src->u.account_login_reply.error.s, src->u.account_login_reply.error.n) == 0) ||
                (sn_memcmp("ok_registered", 13, src->u.account_login_reply.error.s, src->u.account_login_reply.error.n) == 0) ||
                (sn_memcmp("version", 7, src->u.account_login_reply.error.s, src->u.account_login_reply.error.n) == 0) ||
                (sn_memcmp("try_again", 9, src->u.account_login_reply.error.s, src->u.account_login_reply.error.n) == 0) ||
                (sn_memcmp("invalid_login", 13, src->u.account_login_reply.error.s, src->u.account_login_reply.error.n) == 0) ||
                (sn_memcmp("general_failure", 15, src->u.account_login_reply.error.s, src->u.account_login_reply.error.n) == 0) ||
                (sn_memcmp("already_logged", 14, src->u.account_login_reply.error.s, src->u.account_login_reply.error.n) == 0)                 ) {
                    add_intern(pool, dst, src->u.account_login_reply.error.s, src->u.account_login_reply.error.n);
                } else { return -1; }
            break;
        case ACCOUNT_EXISTS:
            add_intern(pool, dst, src->u.account_exists.email.s, src->u.account_exists.email.n);
            add_intern(pool, dst, src->u.account_exists.password.s, src->u.account_exists.password.n);
            break;
        case ACCOUNT_EXISTS_REPLY:
            if (
                (sn_memcmp("ok", 2, src->u.account_exists_reply.error.s, src->u.account_exists_reply.error.n) == 0) ||
                (sn_memcmp("invalid_login", 13, src->u.account_exists_reply.error.s, src->u.account_exists_reply.error.n) == 0) ||
                (sn_memcmp("general_failure", 15, src->u.account_exists_reply.error.s, src->u.account_exists_reply.error.n) == 0)                 ) {
                    add_intern(pool, dst, src->u.account_exists_reply.error.s, src->u.account_exists_reply.error.n);
                } else { return -1; }
            break;
        case VERSION_MISMATCH:
            add_intern(pool, dst, src->u.version_mismatch.master.s, src->u.version_mismatch.master.n);
            add_intern(pool, dst, src->u.version_mismatch.slave.s, src->u.version_mismatch.slave.n);
            break;

        default:
            return -1;
        break;
    }

    return 0;
}

#define CRET(m_func)\
    ret = m_func;\
    if (ret != GCPROTO_OK) return ret;

int gc_deserialize(struct proto_s *dst, sn *src)
{
    src->offset = 0;

    int ret;

    int version;
    CRET(get_int(src, &version))

    if (version != GCPROTO_VERSION) {
        return GCPROTO_ERR_VERSION;
    }

    CRET(get_enum(src, &dst->type))

    switch(dst->type) {
    case MESSAGE_TO:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.message_to.to.s = tmp.s;
        dst->u.message_to.to.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.message_to.address.s = tmp.s;
        dst->u.message_to.address.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.message_to.tp.s = tmp.s;
        dst->u.message_to.tp.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.message_to.body.s = tmp.s;
        dst->u.message_to.body.n = tmp.n;}
        break;
    case MESSAGE_TO_SET_REPLY:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.message_to_set_reply.error.s = tmp.s;
        dst->u.message_to_set_reply.error.n = tmp.n;}
        break;
    case PING_SET:
        break;
    case PONG_SET:
        break;
    case ACCOUNT_LIST:
        break;
    case ACCOUNT_LIST_REPLY:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_list_reply.error.s = tmp.s;
        dst->u.account_list_reply.error.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_list_reply.list.s = tmp.s;
        dst->u.account_list_reply.list.n = tmp.n;}
        break;
    case TRAFFIC_MI:
        break;
    case TRAFFIC_GET:
        break;
    case TRAFFIC_GET_REPLY:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.traffic_get_reply.list.s = tmp.s;
        dst->u.traffic_get_reply.list.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.traffic_get_reply.error.s = tmp.s;
        dst->u.traffic_get_reply.error.n = tmp.n;}
        break;
    case MESSAGE_FROM:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.message_from.from_cloud.s = tmp.s;
        dst->u.message_from.from_cloud.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.message_from.from_device.s = tmp.s;
        dst->u.message_from.from_device.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.message_from.from_address.s = tmp.s;
        dst->u.message_from.from_address.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.message_from.tp.s = tmp.s;
        dst->u.message_from.tp.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.message_from.body.s = tmp.s;
        dst->u.message_from.body.n = tmp.n;}
        break;
    case DEVICE_PAIR:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.device_pair.cloud.s = tmp.s;
        dst->u.device_pair.cloud.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.device_pair.device.s = tmp.s;
        dst->u.device_pair.device.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.device_pair.local_port.s = tmp.s;
        dst->u.device_pair.local_port.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.device_pair.remote_port.s = tmp.s;
        dst->u.device_pair.remote_port.n = tmp.n;}
        break;
    case DEVICE_PAIR_REPLY:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.device_pair_reply.cloud.s = tmp.s;
        dst->u.device_pair_reply.cloud.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.device_pair_reply.error.s = tmp.s;
        dst->u.device_pair_reply.error.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.device_pair_reply.list.s = tmp.s;
        dst->u.device_pair_reply.list.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.device_pair_reply.type.s = tmp.s;
        dst->u.device_pair_reply.type.n = tmp.n;}
        break;
    case OFFLINE_SET:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.offline_set.address.s = tmp.s;
        dst->u.offline_set.address.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.offline_set.cloud.s = tmp.s;
        dst->u.offline_set.cloud.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.offline_set.device.s = tmp.s;
        dst->u.offline_set.device.n = tmp.n;}
        break;
    case ACCOUNT_SET:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_set.email.s = tmp.s;
        dst->u.account_set.email.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_set.password.s = tmp.s;
        dst->u.account_set.password.n = tmp.n;}
        break;
    case ACCOUNT_SET_REPLY:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_set_reply.error.s = tmp.s;
        dst->u.account_set_reply.error.n = tmp.n;}
        break;
    case ACCOUNT_GET:
        break;
    case ACCOUNT_LOGIN:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_login.email.s = tmp.s;
        dst->u.account_login.email.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_login.password.s = tmp.s;
        dst->u.account_login.password.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_login.devname.s = tmp.s;
        dst->u.account_login.devname.n = tmp.n;}
        break;
    case ACCOUNT_LOGIN_REPLY:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_login_reply.error.s = tmp.s;
        dst->u.account_login_reply.error.n = tmp.n;}
        break;
    case ACCOUNT_EXISTS:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_exists.email.s = tmp.s;
        dst->u.account_exists.email.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_exists.password.s = tmp.s;
        dst->u.account_exists.password.n = tmp.n;}
        break;
    case ACCOUNT_EXISTS_REPLY:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.account_exists_reply.error.s = tmp.s;
        dst->u.account_exists_reply.error.n = tmp.n;}
        break;
    case VERSION_MISMATCH:
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.version_mismatch.master.s = tmp.s;
        dst->u.version_mismatch.master.n = tmp.n;}
        { sn tmp; CRET(get_intern(&tmp, src));
        dst->u.version_mismatch.slave.s = tmp.s;
        dst->u.version_mismatch.slave.n = tmp.n;}
        break;

        default:
        return -1;
    }

    return 0;
}
