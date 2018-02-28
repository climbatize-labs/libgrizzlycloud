/*
 *
 * GrizzlyCloud library - simplified VPN alternative for IoT
 * Copyright (C) 2016 - 2017 Filip Pancik
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
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#define RB_SLOT_SIZE    (32 * 1024)

/**
 * @brief Ringbuffer slot specification.
 *
 */
struct ringbuffer_slot_s {
    void   *buf;                    /**< Actual data. */
    int    len;                     /**< Data length. */
    int    sent;                    /**< Amount of data already sent. */
    struct ringbuffer_slot_s *next; /**< Next slot in linked list. */
};

/**
 * @brief Core ringbuffer structure.
 *
 * Used for both, receiving and sending data.
 */
struct ringbuffer_s {
    struct {
        char tmp[RB_SLOT_SIZE];     /**< Temporary buffer to receive data. */
        void *buf;                  /**< Dynamically allocated storage built of tmp parts. */
        int  len;                   /**< Length of storage. */
        int  target;                /**< Target length to receive. */
    } recv;

    struct ringbuffer_slot_s *send; /**< Linked list of slot buffers to send. */
    struct ringbuffer_slot_s *tail; /**< Pointer to last slot in linked list for fast access. */
};

/**
 * @brief Get next buffer that is ready to be sent.
 *
 * @param rb Ringbuffer structure.
 * @param size Size of data.
 * @return Pointer to data.
 */
char *ringbuffer_send_next(struct ringbuffer_s *rb, int *size);

/**
 * @brief Mark data being already sent.
 *
 * @param rb Ringbuffer structure.
 * @param offset Number of bytes already sent.
 * @return void.
 */
void ringbuffer_send_skip(struct ringbuffer_s *rb, int offset);

/**
 * @brief Check if there is anything to send.
 *
 * @param rb Ringbuffer structure.
 * @return 1 if empty, 0 if not empty.
 */
int ringbuffer_send_is_empty(struct ringbuffer_s *rb);

/**
 * @brief Clear send buffers.
 *
 * @param rb Ringbuffer structure.
 * @return void.
 */
void ringbuffer_send_pop_all(struct ringbuffer_s *rb);

/**
 * @brief Append data for sending.
 *
 * @param rb Ringbuffer structure.
 * @param buf Data pointer.
 * @param len Length of data.
 * @return GC_OK on success, GC_ERROR on failure.
 */
int ringbuffer_send_append(struct ringbuffer_s *rb, char *buf, const int len);

/**
 * @brief Total bytes to send.
 *
 * @param rb Ringbuffer structure.
 * @return Number of bytes.
 */
int ringbuffer_send_size(struct ringbuffer_s *rb);

/**
 * @brief Append received data.
 *
 * Take data from temporary buffer and copy them to storage.
 *
 * @param rb Ringbuffer structure.
 * @param len Data length.
 * @return Number of bytes.
 */
void ringbuffer_recv_append(struct ringbuffer_s *rb, const int len);

/**
 * @brief Obtain received data.
 *
 * @param rb Ringbuffer structure.
 * @param size Size of received data.
 * @return Pointer to received data.
 */
char *ringbuffer_recv_read(struct ringbuffer_s *rb, int *size);

/**
 * @brief Release received data.
 *
 * @param rb Ringbuffer structure.
 * @return void.
 */
void ringbuffer_recv_pop(struct ringbuffer_s *rb);

/**
 * @brief Check if buffer is full.
 *
 * @param rb Ringbuffer structure.
 * @return 1 if full, 0 if not full.
 */
int ringbuffer_recv_is_full(struct ringbuffer_s *rb);

#endif
