/*
 * bg_album.c
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

///////////////////////////////////////////////////////////////////////////////
static bg_tree_vmt_t bg_album_vmt;

int bg_album_create(bg_tree_t **tree, bg_param_t *param, bg_tree_t *parent,
    const ffchar_t *path)
{
  /////////////////////////////////////////////////////////////////////////////
  *tree=malloc(sizeof **tree);

  if (!*tree) {
    _DMESSAGE("allocating");
    goto e_alloc;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_tree_common_create(*tree,param,parent,path)<0) {
    _DMESSAGE("creating common");
    goto e_common;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_album_content_create(*tree,&bg_album_vmt)<0) {
    _DMESSAGE("creating content");
    goto e_content;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (parent&&bg_album_push(parent,*tree)<0) {
    _DMESSAGE("pushing");
    goto e_push;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  if (parent)
    bg_album_pop(parent);
e_push:
  bg_album_content_destroy(*tree);
e_content:
  bg_tree_common_destroy(*tree);
e_common:
  free(*tree);
e_alloc:
  return -1;
}

void bg_album_destroy(bg_tree_t *tree)
{
  if (tree->parent&&tree!=bg_album_pop(tree->parent))
    _DWARNING("tree not at end of list");

  if (!tree->param->process&&1u<tree->album.nchildren.max) {
    // it's getting bottom-up.
    if (!tree->param->process)
      tree->argv->lift=tree->depth+1;
  }

  bg_album_content_destroy(tree);
  bg_tree_common_destroy(tree);
  free(tree);
}

int bg_album_content_create(bg_tree_t *tree, bg_tree_vmt_t *vmt)
{
  bg_album_t *album=&tree->album;

  tree->vmt=vmt;
  album->nchildren.max=0u;
  album->nchildren.cur=0u;
  album->nleafs=0u;
#if defined (BG_TRACK_ID) // [
  album->track.id=0u;
#endif // ]
  album->first=NULL;
  album->last=NULL;

  return 0;
}

void bg_album_content_destroy(bg_tree_t *tree)
{
  bg_tree_t *cur;

  while (tree->album.last) {
    cur=bg_album_pop(tree);

    if (cur)
      cur->vmt->destroy(cur);
    else
      _DWARNING("empty list");
  }
}

///////////////////////////////////////////////////////////////////////////////
int bg_album_push(bg_tree_t *tree, bg_tree_t *child)
{
  bg_album_t *album=&tree->album;

  if (album->last) {
    child->prev=album->last;
    album->last->next=child;
  }
  else
    album->first=child;

  album->last=child;
#if defined (BG_TRACK_ID) // [
  child->vmt->track_id(child,&album->track.id);
#endif // ]
  ++album->nchildren.max;
  ++album->nchildren.cur;

  return 0;
}

bg_tree_t *bg_album_pop(bg_tree_t *tree)
{
  bg_album_t *album=&tree->album;
  bg_tree_t *last=album->last;

  --album->nchildren.cur;

  if (last) {
    last->parent=NULL;
    album->last=last->prev;

    if (album->last)
      album->last->next=NULL;
    else
      album->first=NULL;
  }
  else
    album->first=NULL;


  return last;
}

///////////////////////////////////////////////////////////////////////////////
static int bg_album_accept(bg_tree_t *tree, bg_visitor_t *vis)
{
  return vis->vmt->dispatch_album(vis,tree);
}

#if defined (BG_TRACK_ID) // [
static void bg_album_track_id(bg_tree_t *tree FFUNUSED, int *id FFUNUSED)
{
}
#endif // ]

static bg_tree_vmt_t bg_album_vmt={
  .id=FFL("album"),
  .type=BG_TREE_TYPE_ALBUM,
  .destroy=bg_album_destroy,
  .accept=bg_album_accept,
  .annotation={
    .create=bg_album_annotation_create,
    .destroy=bg_album_annotation_destroy,
  },
#if defined (BG_TRACK_ID) // [
  .track_id=bg_album_track_id,
#endif // ]
};
