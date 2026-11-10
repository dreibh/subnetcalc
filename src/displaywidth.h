// ==========================================================================
//             ___  _         _         __      ___    _ _   _
//            |   \(_)____ __| |__ _ _  \ \    / (_)__| | |_| |_
//            | |) | (_-< '_ \ / _` | || \ \/\/ /| / _` |  _| ' \
//            |___/|_/__/ .__/_\__,_|\_, |\_/\_/ |_\__,_|\__|_||_|
//                      |_|          |__/
// ==========================================================================
//
// Display Width Computation
// Copyright (C) 2026 by Thomas Dreibholz
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Contact: thomas.dreibholz@gmail.com

#ifndef DISPLAYWIDTH_H
#define DISPLAYWIDTH_H

#include <cstddef>
#include <string_view>

size_t skipAnsiSequence(std::string_view sv, size_t offset) noexcept;
size_t computeDisplayWidth(std::string_view sv) noexcept;

#endif // DISPLAYWIDTH_H
