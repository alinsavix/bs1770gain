/*
 * ffsox_machine.c

 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#include <ffsox_priv.h>

int ffsox_machine_run(machine_t *m, node_t *node)
{
  int op;

  m->node=node;

  while (NULL!=(node=m->node)) {
//fprintf(stderr,"%s: RUN\n",node->vmt->name);
    if ((op=node->vmt->run(node))<0) {
      MESSAGE("running machine");
      return op;
    }

    switch (op) {
    case MACHINE_PUSH:
//fprintf(stderr,"%s: PUSH\n",node->vmt->name);
      m->node=node->vmt->next(node);
      break;
    case MACHINE_POP:
//fprintf(stderr,"%s: POP\n",node->vmt->name);
      m->node=node->vmt->prev(node);
      break;
    default:
      MESSAGE("illegal instruction");
      return -1;
    }
  }

  return 0;
}
