/*
 * bg_rm.c
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
#include <ff.h>

int ff_rm(const ffchar_t *path)
{
#if defined (_WIN32) // [
  DWORD dwError;

  if (!DeleteFileW(path)) {
    dwError=GetLastError();

    switch (dwError) {
    default:
#if defined (_WIN32) // [
      _DMESSAGEV("removing \"%S\" (%lu)",path,dwError);
#else // ] [
      _DMESSAGEV("removing \"%s\" (%lu)",path,dwError);
#endif // ]
      break;
    }

    goto e_remove;
  }
#else // ] [
  if (remove(path)<0) {
#if defined (_WIN32) // [
    _DMESSAGEV("removing \"%S\"",path);
#else // ] [
    _DMESSAGEV("removing \"%s\"",path);
#endif // ]
    goto e_remove;
  }
#endif // ]
  return 0;
//cleanup:
e_remove:
  return -1;
}
