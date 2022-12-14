/*
 * bg_basenamea
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
#include <bg.h>

#if defined (_WIN32) // [
const char *bg_basenamea(const char *path)
{
  const char *bn=path;
  const char *basename;

  if (1<strlen(bn)&&':'==bn[1]) {
    bn+=2;

    if ('\\'==*bn)
      ++bn;
  }

  basename=bn;

  for (;;) {
    bn=strstr(bn,"\\");

    if (bn)
      basename=++bn;
    else
      break;
  }

  return basename;
}
#endif // ]
