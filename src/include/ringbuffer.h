#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#define RB_SLOT_SIZE    (32 * 1024)

struct ringbuffer_slot_s {
    void *buf;
    int len;
    int sent;
    struct ringbuffer_slot_s *next;
};

struct ringbuffer_s {
    struct {
        char tmp[RB_SLOT_SIZE];
        void *buf;
        int len;
        int target;
    } recv;

    struct ringbuffer_slot_s *send, *tail;
};

char *ringbuffer_send_next(struct ringbuffer_s *rb, int *size);
void ringbuffer_send_skip(struct ringbuffer_s *rb, int offset);
int ringbuffer_send_is_empty(struct ringbuffer_s *rb);
void ringbuffer_send_pop(struct ringbuffer_s *rb);
void ringbuffer_send_pop_all(struct ringbuffer_s *rb);
int ringbuffer_send_append(struct ringbuffer_s *rb, char *buf, const int len);
char *ringbuffer_recv_ptr(struct ringbuffer_s *rb, int *used);
void ringbuffer_recv_append(struct ringbuffer_s *rb, const int len);
char *ringbuffer_recv_read(struct ringbuffer_s *rb, int *size);
void ringbuffer_recv_pop(struct ringbuffer_s *rb);
int ringbuffer_recv_is_full(struct ringbuffer_s *rb);

int ringbuffer_send_size(struct ringbuffer_s *rb);

#endif
