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
#ifndef GC_FS_H_
#define GC_FS_H_

/**
 * @brief Announce successful pair to the filesystem.
 *
 * @param log Log structure.
 * @param pair Pair structure.
 * @return void.
 */
void fs_pair(struct hm_log_s *log, struct gc_device_pair_s *pair);

/**
 * @brief Remove paired record from the filesystem.
 *
 * @param log Log structure.
 * @param pid Binary pid.
 * @return void.
 */
void fs_unpair(struct hm_log_s *log, snb *pid);

#endif
