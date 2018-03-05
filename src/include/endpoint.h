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
#ifndef GC_ENDPOINT_H_
#define GC_ENDPOINT_H_

/**
 * @brief Endpoint representation.
 *
 * Specifies endpoint along with local tcp client.
 */
struct gc_endpoint_s {
    snb key;                        /**< Key to identify endpoint.*/
    snb remote_fd;                  /**< Remote file descriptor. */
    snb backend_port;               /**< Backend port. */
    snb pid;                        /**< Process ID association. */

    struct gc_gen_client_s *client;   /**< TCP client. */

    struct gc_endpoint_s *next;     /**< Next endpoint in linked list. */
};

/**
 * @brief Handle endpoint request.
 *
 * @param gc GC structure.
 * @param p Proto message.
 * @param argv Array of parsed header elements.
 * @param argv Number of elements in array.
 * @return GC_OK on success, GC_ERROR on failure.
 */
int gc_endpoint_request(struct gc_s *gc, struct proto_s *p, char **argv, int argc);

/**
 * @brief Stop endpoint.
 *
 * @param log Logging stream.
 * @param address Process ID.
 * @param cloud Cloud name.
 * @param device Device name.
 * @return void.
 */
void gc_endpoint_stop(struct hm_log_s *log, sn address, sn cloud, sn device);

/**
 * @brief Stop all endpoints.
 *
 * @return void.
 */
void gc_endpoints_stop_all();

#endif
