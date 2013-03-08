/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2012  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ocland.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

/** Set log file destination.
 * @param path file path.
 * @return 1 if file could opened,
 * 0 otherwise.
 */
int setLogFile(const char* path);

/** Initialize log tool. This method
 * only ensures that setLogFile has
 * been called, calling it if not.
 * You must call this method ever
 * before starting work.
 * @return 1 if file could opened,
 * 0 otherwise.
 * @remarks If log file has not been
 * previously set, /var/log/ocland.log
 * will set.
 */
int initLog();

#endif // LOG_H_INCLUDED
