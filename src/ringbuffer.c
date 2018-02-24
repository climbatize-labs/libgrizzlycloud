#include <gc.h>

/*
 * Obtain pointer to data and its size
 *
 * Warning: this is not aligned
 */
char *ringbuffer_send_next(struct ringbuffer_s *rb, int *size)
{
    assert(rb);

    if(rb->send == NULL) {
        *size = 0;
        return NULL;
    }

    *size = rb->send->len - rb->send->sent;

    return (char *)(rb->send->buf + rb->send->sent);
}

/*
 * @static
 * Remove empty send buffer if it's already been sent
 *
 */
static void ringbuffer_next(struct ringbuffer_s *rb)
{
    struct ringbuffer_slot_s *next;

    if(rb && rb->send && rb->send->sent == rb->send->len) {
        next = rb->send->next;
        free(rb->send->buf);
        free(rb->send);
        rb->send = next;
        if(rb->send == NULL) {
            rb->tail = NULL;
        }
    }
}

/*
 * Specify how many data were actually sent
 *
 */
void ringbuffer_send_skip(struct ringbuffer_s *rb, int offset)
{
    assert(rb);
    assert(rb->send);
    rb->send->sent += offset;

    ringbuffer_next(rb);
}

/*
 * Find out if there is anything to send
 *
 */
int ringbuffer_send_is_empty(struct ringbuffer_s *rb)
{
    assert(rb);
    return (rb->send == NULL);
}

/*
 * Wipe out entire list of send buffers
 *
 */
void ringbuffer_send_pop_all(struct ringbuffer_s *rb)
{
    struct ringbuffer_slot_s *r, *rdel;

    for(r = rb->send; r != NULL; ) {
        free(r->buf);
        rdel = r;
        r = r->next;
        free(rdel);
    }
}

/*
 * Traverse list of send buffers to find out size
 *
 */
int ringbuffer_send_size(struct ringbuffer_s *rb)
{
    int size = 0;
    struct ringbuffer_slot_s *r;

    for(r = rb->send; r != NULL; r = r->next) {
        size += r->len;
    }

    return size;
}

/*
 * Add data for sending
 *
 */
int ringbuffer_send_append(struct ringbuffer_s *rb, char *buf, const int len)
{
    assert(rb);
    struct ringbuffer_slot_s *slot;

    slot = malloc(sizeof(*slot));
    if(slot == NULL) {
        return GC_ERROR;
    }
    slot->buf = malloc(len);
    if(slot->buf == NULL) {
        free(slot);
        return GC_ERROR;
    }

    memcpy(slot->buf, buf, len);
    slot->len = len;
    slot->sent = 0;
    slot->next = NULL;

    if(rb->send == NULL && rb->tail == NULL) {
        rb->send = rb->tail = slot;
    } else {
        assert(rb->tail);
        rb->tail->next = slot;
        rb->tail = slot;
    }

    return GC_OK;
}

void ringbuffer_recv_append(struct ringbuffer_s *rb, const int len)
{
    assert(rb);
    rb->recv.buf = realloc(rb->recv.buf, rb->recv.len + len);
    memcpy(rb->recv.buf + rb->recv.len, rb->recv.tmp, len);
    rb->recv.len += len;
}

void ringbuffer_recv_pop(struct ringbuffer_s *rb)
{
    assert(rb);
    rb->recv.len = 0;
    rb->recv.target = 0;
    if(rb->recv.buf) {
        free(rb->recv.buf);
        rb->recv.buf = NULL;
    }
}

char *ringbuffer_recv_read(struct ringbuffer_s *rb, int *size)
{
    *size = rb->recv.len;
    return rb->recv.buf;
}

int ringbuffer_recv_is_full(struct ringbuffer_s *rb)
{
    assert(rb);
    return GC_OK;
    //return(rb->recv.len >= RB_SLOT_SIZE);
}
