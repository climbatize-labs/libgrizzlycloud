// This file is generated 
#ifndef GC_PROTO_H_
#define GC_PROTO_H_

#define GCPROTO_VERSION	1
#define GCPROTO_OK  0
#define GCPROTO_ERR 1
#define GCPROTO_ERR_VERSION 2

enum proto_e {
    MESSAGE_TO_SET_REPLY,
    MESSAGE_TO,
    ACCOUNT_LIST,
    ACCOUNT_LIST_REPLY,
    TRAFFIC_MI,
    TRAFFIC_GET,
    TRAFFIC_GET_REPLY,
    MESSAGE_FROM,
    DEVICE_PAIR,
    DEVICE_PAIR_REPLY,
    OFFLINE_SET,
    ACCOUNT_SET,
    ACCOUNT_SET_REPLY,
    ACCOUNT_GET,
    ACCOUNT_LOGIN,
    ACCOUNT_LOGIN_REPLY,
    ACCOUNT_EXISTS,
    ACCOUNT_EXISTS_REPLY,
    VERSION_MISMATCH,
};

struct proto_s {
    enum proto_e type;
    union {
        struct {
            sn     error;
        } message_to_set_reply;
        struct {
            sn     to;
            sn     address;
            sn     tp;
            sn     body;
        } message_to;
        struct {
            /* void */
        } account_list;
        struct {
            sn     error;
            sn     list;
        } account_list_reply;
        struct {
            /* void */
        } traffic_mi;
        struct {
            /* void */
        } traffic_get;
        struct {
            sn     list;
            sn     error;
        } traffic_get_reply;
        struct {
            sn     from_cloud;
            sn     from_device;
            sn     from_address;
            sn     tp;
            sn     body;
        } message_from;
        struct {
            sn     cloud;
            sn     device;
            sn     local_port;
            sn     remote_port;
        } device_pair;
        struct {
            sn     cloud;
            sn     error;
            sn     list;
            sn     type;
        } device_pair_reply;
        struct {
            sn     address;
            sn     cloud;
            sn     device;
        } offline_set;
        struct {
            sn     email;
            sn     password;
        } account_set;
        struct {
            sn     error;
        } account_set_reply;
        struct {
            /* void */
        } account_get;
        struct {
            sn     email;
            sn     password;
            sn     devname;
        } account_login;
        struct {
            sn     error;
        } account_login_reply;
        struct {
            sn     email;
            sn     password;
        } account_exists;
        struct {
            sn     error;
        } account_exists_reply;
        struct {
            sn     master;
            sn     slave;
        } version_mismatch;
    } u;
};
int gc_serialize(sn *dst, struct proto_s *src);
int gc_deserialize(struct proto_s *dst, sn *src);
void gc_proto_dump(struct proto_s *p);


#endif