/*
 * ff_fexists.c
 *


 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <ff.h>

int ff_fexists(const ffchar_t *path)
{
  struct stat buf;

#if (_WIN32) // [
  return !wstat(path,&buf);
#else // ] [
  return !stat(path,&buf);
#endif // ]
}
