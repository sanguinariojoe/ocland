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

#ifndef WIN32
    #include <unistd.h>
#else
    #include <windows.h>

    static int usleep(unsigned int usec)
    {
        LARGE_INTEGER relative_time = { 0 };
        relative_time.QuadPart = -10 * (int)usec;

        HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
        SetWaitableTimer(timer, &relative_time, 0, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
        return 0;
    }
#endif

#endif /* USLEEP */
