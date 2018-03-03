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
#ifndef GC_BACKEND_H_
#define GC_BACKEND_H_

/**
 * @brief Initialize backend.
 *
 * Pick one of the available backend nodes.
 *
 * @param gc GC structure.
 * @param chosen Selected backend node.
 * @return GC_OK on success, GC_ERROR on failure
 */
int gc_backend_init(struct gc_s *gc, snb *chosen);

#endif
