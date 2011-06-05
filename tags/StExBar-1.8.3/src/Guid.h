// StExBar - an explorer toolbar

// Copyright (C) 2007-2008 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#pragma once

// the GUIDs used in this COM-Object

#ifdef _WIN64
// {6C7A85A7-27C6-49ce-98B2-A8479B0DD63D}
DEFINE_GUID(CLSID_StExBand,
            0x6c7a85a7, 0x27c6, 0x49ce, 0x98, 0xb2, 0xa8, 0x47, 0x9b, 0xd, 0xd6, 0x3d);
#else
// {367D8B32-F9FD-474b-8E65-9E521F35DE99}
DEFINE_GUID(CLSID_StExBand,
            0x367d8b32, 0xf9fd, 0x474b, 0x8e, 0x65, 0x9e, 0x52, 0x1f, 0x35, 0xde, 0x99);
#endif
