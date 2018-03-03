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
#ifndef GC_TUNNEL_H_
#define GC_TUNNEL_H_

/**
 * @brief Tunnel representation.
 *
 * Specifies tunnel along with local tcp server.
 */
struct gc_tunnel_s {
    snb    cloud;                   /**< Cloud. */
    snb    pid;                     /**< Pid. */
    snb    device;                  /**< Device name. */
    snb    port_local;              /**< Local port. */
    snb    port_remote;             /**< Remote port. */
    snb    type;                    /**< Type of tunnel. */

    int    active;                  /**< Sign of activity. */

    struct conn_server_s *server;   /**< Local TCP server related with tunnel. */

    struct gc_tunnel_s *next;       /**< Pointer to next tunnel in a linked list. */
};

/**
 * @brief Create new tunnel.
 *
 * Create a local TCP listener if @p type doesn't equal to "forced".
 * Upstream forces us to create new tunnel when some other user
 * establishes connection with our user.
 *
 * @param gc GC structure.
 * @param pair Device pair structure.
 * @param type Type of tunnel to add.
 * @return GC_OK on success, GC_ERROR on failure.
 */
int gc_tunnel_add(struct gc_s *gc, struct gc_device_pair_s *pair, sn type);

/**
 * @brief Handle tunnel response.
 *
 * Tunnel sends request to specific cloud via upstream.
 * When cloud response arrives, we handle such message here.
 *
 * @param gc GC structure.
 * @param p Message to deliver to our client that made request earlier.
 * @param argv Array of parsed header elements.
 * @param argc Number of elements in array.
 * @return GC_OK on success, GC_ERROR on failure.
 */
int gc_tunnel_response(struct gc_s *gc, struct proto_s *p, char **argv, int argc);

/**
 * @brief Stop tunnel.
 *
 * @param pid process ID
 * @return void.
 */
void gc_tunnel_stop(sn pid);

/**
 * @brief Stop all tunnels.
 *
 * @return void.
 */
void gc_tunnel_stop_all();

#endif
