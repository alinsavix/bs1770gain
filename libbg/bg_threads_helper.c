/*
 * bg_threads_helper.c
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

#if defined (BG_PARAM_THREADS) // [
int bg_threads_helper_create(bg_threads_helper_t *helper)
{
  if (bg_sync_create(&helper->sync)<0) {
    _DMESSAGE("creating sync");
    goto e_sync;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  bg_sync_destroy(&helper->sync);
e_sync:
  return -1;
}

void bg_threads_helper_destroy(bg_threads_helper_t *helper)
{
  bg_sync_destroy(&helper->sync);
}
#endif // ]
