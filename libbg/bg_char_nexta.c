/*
 * charnexta.c
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

#if defined (BG_CHAR_NEXTA) // [
enum {
  // cf. e.g. "https://en.wikipedia.org/wiki/UTF-8#Description".
  MASK_ONE_BYTE=~(1u<<7u),
  MASK_TWO_BYTES=~(1u<<5u),
  MASK_THREE_BYTES=~(1u<<4u),
  MASK_FOUR_BYTES=~(1u<<3u),
};

const char *bg_char_nexta(const char *str)
{
  if (*(const uint8_t *)str==(MASK_ONE_BYTE&*(const uint8_t *)str))
    return str+1;
  else if (*(const uint8_t *)str==(MASK_TWO_BYTES&*(const uint8_t *)str))
    return str+2;
  else if (*(const uint8_t *)str==(MASK_THREE_BYTES&*(const uint8_t *)str))
    return str+3;
  else if (*(const uint8_t *)str==(MASK_FOUR_BYTES&*(const uint8_t *)str))
    return str+4;
  else {
    DVMESSAGE("unexpected byte: %02x",*str);
    exit(1);
    return NULL;
  }
}
#endif // ]
