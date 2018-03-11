#include <gc.h>

int main()
{
    struct hm_pool_s *pool;
    struct hm_log_s glog;

    struct gc_ringbuffer_s rb;
    char *buf = "aaaaBBBB";

    hm_log_open(&glog, NULL, LOG_TRACE);
    pool = hm_create_pool(&glog);

    if(pool == NULL) return 1;

    memset(&rb, 0, sizeof(rb));

    int i;
    for(i = 0; i < 10; i++) {
        gc_ringbuffer_recv_append(pool, &rb, strlen(buf));
    }

    gc_ringbuffer_recv_pop(pool, &rb);

    /* send */

    for(i = 0; i < 10; i++) {
        gc_ringbuffer_send_append(pool, &rb, buf, strlen(buf));
    }

    printf("%d\n", gc_ringbuffer_send_size(&rb));

    while(!gc_ringbuffer_send_is_empty(&rb)) {
        int size;
        char *next = gc_ringbuffer_send_next(&rb, &size);
        (void )next;
        // Simulate network send()
        // where arbitrary bytes can be written to socket
        // so we only send half of the required size
        size = size > 1 ? size / 2 : 1;
        gc_ringbuffer_send_skip(pool, &rb, size);
    }

    gc_ringbuffer_send_pop_all(pool, &rb);

    hm_destroy_pool(pool);

    return 0;
}
