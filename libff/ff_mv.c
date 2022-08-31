/*
 * ff_mv.c
 *
 * Copyright (C) 2019 Peter Belkner <info@pbelkner.de>
 * Nanos gigantum humeris insidentes #TeamWhite
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

int ff_mv(const ffchar_t *source, const ffchar_t *target)
{
#if defined (_WIN32) // [
  DWORD dwError;

  DeleteFileW(target);

  if (!MoveFileW(source,target)) {
    dwError=GetLastError();

    switch (dwError) {
    case ERROR_ALREADY_EXISTS:
#if defined (_WIN32) // [
      _DMESSAGEV("moving \"%S\" to \"%S\" (ERROR_ALREADY_EXISTS)",
          source,target);
#else // ] [
      _DMESSAGEV("moving \"%s\" to \"%s\" (ERROR_ALREADY_EXISTS)",
          source,target);
#endif // ]
      break;
    default:
#if defined (_WIN32) // [
      _DMESSAGEV("moving \"%S\" to \"%S\" (%lu)",source,target,dwError);
#else // ] [
      _DMESSAGEV("moving \"%s\" to \"%s\" (%lu)",source,target,dwError);
#endif // ]
      break;
    }

    goto e_rename;
  }
#else // ] [
  remove(target);

  if (rename(source,target)<0) {
#if defined (_WIN32) // [
    _DMESSAGEV("moving \"%S\" to \"%S\"",source,target);
#else // ] [
    _DMESSAGEV("moving \"%s\" to \"%s\"",source,target);
#endif // ]
    goto e_rename;
  }
#endif // ]
  return 0;
//cleanup:
e_rename:
  return -1;
}
