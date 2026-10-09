#!/usr/bin/env bash
#
# Build Scripts
# Copyright (C) 2002-2026 by Thomas Dreibholz
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Contact: thomas.dreibholz@gmail.com

# Bash options:
set -euo pipefail

CMAKE_OPTIONS=()
COMMAND=""
CORES=0

while [ $# -gt 0 ] ; do
   if [[ "$1" =~ ^(-|--)use-clang$ ]] ; then
      export CXX=clang++
      export CC=clang
   elif [[ "$1" =~ ^(-|--)use-clang-scan-build$ ]] ; then
      export CXX=clang++
      export CC=clang
      mkdir -p scan-build-reports
      COMMAND="scan-build -o scan-build-reports"
   elif [[ "$1" =~ ^(-|--)use-gcc$ ]] ; then
      export CXX=g++
      export CC=gcc
   elif [[ "$1" =~ ^(-|--)use-gcc-analyzer$ ]] ; then
      export CXX=g++
      export CC=gcc
      export CFLAGS=-fanalyzer
      export CXXFLAGS=-fanalyzer
      CMAKE_OPTIONS+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
   elif [[ "$1" =~ ^(-|--)debug$ ]] ; then
      CMAKE_OPTIONS+=("-DCMAKE_BUILD_TYPE=Debug")
   elif [[ "$1" =~ ^(-|--)release$ ]] ; then
      CMAKE_OPTIONS+=("-DCMAKE_BUILD_TYPE=Release")
   elif [[ "$1" =~ ^(-|--)release-with-debinfo$ ]] ; then
      CMAKE_OPTIONS+=("-DCMAKE_BUILD_TYPE=RelWithDebInfo")
   elif [[ "$1" =~ ^(-|--)verbose$ ]] ; then
      CMAKE_OPTIONS+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
   elif [[ "$1" =~ ^(-|--)cores ]] ; then
      if [ $# -lt 2 ] || [[ ! "$2" =~ ^[0-9]+$ ]] ; then
         echo >&2 "ERROR: Number of cores must be an integer number!"
         exit 1
      fi
      CORES="$2"
      shift
   elif [ "$1" == "--" ] ; then
      shift
      break
   else
      echo >&2 "Usage: autogen.sh [--use-clang|--use-clang-scan-build|--use-gcc|--use-gcc-analyzer] [--debug|--release|--release-with-debinfo] [--cores N] [--verbose] -- (further CMake/Configure options)"
      exit 1
   fi
   shift
done

UNAME="$(uname)"
case "${UNAME}" in
   Linux|SunOS|GNU)
      installPrefix="/usr"
      ;;
   NetBSD)
      installPrefix="/usr/pkg"
      ;;
   *)
      installPrefix="/usr/local"
      ;;
esac

# ====== Configure with CMake ===============================================
if [ -e CMakeLists.txt ] ; then
   rm -f CMakeCache.txt

   # On Windows, add VCPKG path if available:
   if [[ "${UNAME}" =~ ^(CYGWIN.*|MINGW.*|MSYS.*|Windows_NT)$ ]] ; then
      VCPKG_ROOT="${VCPKG_INSTALLATION_ROOT:-C:/vcpkg}"
      CMAKE_OPTIONS+=("-DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
   fi

   # Append extra CMake arguments passed after "--":
   if [ $# -gt 0 ] ; then
      CMAKE_OPTIONS+=("$@")
   fi

   CMAKE_OPTIONS+=("-DCMAKE_INSTALL_PREFIX=${installPrefix}")

   echo "Executing: ${COMMAND} cmake ${CMAKE_OPTIONS[*]} ."
   ${COMMAND} cmake "${CMAKE_OPTIONS[@]}" .

# ====== Configure with AutoConf/AutoMake ===================================
elif [ -e bootstrap ] ; then
   ./bootstrap
   ./configure "$@"

else
   echo >&2 "ERROR: Failed to configure with CMake or AutoMake/AutoConf!"
   exit 1
fi

# ====== Obtain number of cores =============================================
if [ "${CORES}" -lt 1 ] ; then
   case "${UNAME}" in
      Linux|GNU)
         CORES="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo "1")"
         ;;
      FreeBSD)
         CORES="$(sysctl -n hw.ncpu 2>/dev/null || echo "1")"
         ;;
      NetBSD|OpenBSD)
         CORES="$(/sbin/sysctl -n hw.ncpuonline 2>/dev/null || echo "1")"
         ;;
      SunOS)
         CORES="$(psrinfo -t 2>/dev/null || echo "1")"
         ;;
      Darwin)
         CORES="$(sysctl -n machdep.cpu.core_count 2>/dev/null || echo "1")"
         ;;
      CYGWIN*|MINGW*|MSYS*|Windows_NT)
         CORES="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo "${NUMBER_OF_PROCESSORS:-1}")"
         ;;
      *)
         CORES=1
         ;;
   esac
   echo "This system has ${CORES} cores!"
fi

# ====== Build ==============================================================
echo "Starting build using up to ${CORES} cores ..."
if [[ ! "${UNAME}" =~ ^(CYGWIN.*|MINGW.*|MSYS.*|Windows_NT)$ ]] ; then
   ${COMMAND} make -j "${CORES}"
else
   cmake --build . -j "${CORES}"
fi
