/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2015  Timur Magomedov <tim239@yandex.ru>
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

/** @file
 * @brief usleep() compatibility helper file.
 *
 * This file provides usleep() implemented using win api.
 */

#ifndef USLEEP_H_INCLUDED
#define USLEEP_H_INCLUDED

#ifdef WIN32
    typedef unsigned int useconds_t;
    static int usleep(useconds_t usec)
    {
        // TODO: implement
        return -1;
    }
#else
    #include <unistd.h>
#endif

#endif /* USLEEP */
