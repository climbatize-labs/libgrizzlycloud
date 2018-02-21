#include <stdio.h>
#include <memory.h>

#include <ringbuffer.h>

int main()
{
    struct ringbuffer_s rb;
    char *buf = "aaaaBBBB";

    memset(&rb, 0, sizeof(rb));

    int i;
    for(i = 0; i < 10; i++) {
        ringbuffer_recv_append(&rb, strlen(buf));
    }

    ringbuffer_recv_pop(&rb);

    /* send */

    for(i = 0; i < 10; i++) {
        ringbuffer_send_append(&rb, buf, strlen(buf));
    }

    // printf("%d\n", ringbuffer_send_size(&rb));

    while(!ringbuffer_send_is_empty(&rb)) {
        int size;
        char *next = ringbuffer_send_next(&rb, &size);
        (void )next;
        // Simulate network send()
        // where arbitrary bytes can be written to socket
        // so we only send half of the required size
        size = size > 1 ? size / 2 : 1;
        ringbuffer_send_skip(&rb, size);
    }

    ringbuffer_send_pop_all(&rb);

    return 0;
}
