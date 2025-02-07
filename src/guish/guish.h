/*
 * GuiSH - A Shell for Wayland Programs
 * Copyright (C) 2024  Marcus Harrison
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GUISH_GUISH
#define GUISH_GUISH

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 */

/**
 * \brief Constant file descriptor for the GUI Server.
 *
 * This represents an already-connected socket to the
 * Wayland compositor.
 */
#define GUISRV_FILENO 3

/**
 * \brief Constant file descriptor for a GUI Client.
 *
 * This file descriptor can listen for Wayland clients.
 *
 * If no clients are defined in the pipeline, this is
 * still a socket that can be listened on, but there
 * is no way for clients to connect to it.
 */
#define GUICLI_FILENO 4

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GUISH_GUISH
