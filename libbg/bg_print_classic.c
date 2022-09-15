/*
 * bg_print_classic.c
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

static int bg_print_conf_len(bg_print_conf_t *c, bg_tree_t *tree FFUNUSED)
{
#if defined (_WIN32) // [
  return wcslen(c->w.label);
#else // ] [
  return strlen(c->n.label);
#endif // ]
}

static void bg_print_conf_tail(bg_print_conf_t *c, bg_tree_t *tree,
    int width, FILE *f)
{
  enum { FORMAT_SIZE=128 };

  struct {
#if defined (_WIN32) // [
    wchar_t w[FORMAT_SIZE];
#else // ] [
    char n[FORMAT_SIZE];
#endif // ]
  } format;

#if defined (_WIN32) // [
  _snwprintf(format.w,FORMAT_SIZE,c->w.format.classic,width);
#else // ] [
  snprintf(format.n,FORMAT_SIZE,c->n.format.classic,width);
#endif // ]

  switch (c->argc) {
  case 1:
    if (c->argv[0]) {
#if defined (_WIN32) // [
      fwprintf(f,format.w,c->w.label,c->argv[0](tree),c->n.unit(tree));
#else // ] [
      fprintf(f,format.n,c->n.label,c->argv[0](tree),c->n.unit(tree));
#endif // ]
    }
    else
      _DWARNING("argv[0]");

    break;
  case 2:
    if (c->argv[0]&&c->argv[1]) {
#if defined (_WIN32) // [
      fwprintf(f,format.w,c->n.label,c->argv[0](tree),c->n.unit(tree),
          c->argv[1](tree),c->n.unit(tree));
#else // ] [
      fprintf(f,format.n,c->n.label,c->argv[0](tree),c->n.unit(tree),
          c->argv[1](tree),c->n.unit(tree));
#endif // ]
    }
    else
      _DWARNING("argv[0]/argv[1]");

    break;
  default:
    _DWARNING("argc");
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
static void bg_print_classic_encoding(bg_param_t *param, int bits)
{
  param->result.bits=bits;
}

static int bg_print_classic_width(bg_tree_t *tree)
{
  // path 1:  determine the maximum length of the involved labels.
  int width=-1;
  bg_flags_agg_t agg;
  bg_print_conf_t *c;
  int len;

  for (agg=1,c=bg_print_conf;agg<BG_FLAGS_AGG_MAX;agg<<=1,++c) {
    if (!(agg&tree->param->flags.aggregate)) {
      // this aggregation isn't involved.
      continue;
    }
    else if (agg!=c->aggregate) {
      // wrong order.
      _DWARNING("aggregate mismatch");
      continue;
    }
    else if ((len=bg_print_conf_len(c,tree))<0) {
      _DWARNING("getting length");
      continue;
    }
    else if (width<len)
      width=len;
  }

  return width;
}

static int bg_print_classic_head(bg_tree_t *tree, int depth FFUNUSED, FILE *f)
{
  bg_track_t *track;

#if defined (BG_PARAM_QUIET) // [
  if (tree->param->quiet)
    goto success;
#endif // ]

  switch (tree->vmt->type) {
  case BG_TREE_TYPE_FILE:
    // nothing to do.
    break;
  case BG_TREE_TYPE_TRACK:
    track=&tree->track;
    _FPRINTFV(f,"[%lu/%lu] %s\n",track->root.id,tree->param->count.max,
        bg_tree_in_basename(tree));

    break;
  case BG_TREE_TYPE_ALBUM:
    if (!tree->param->suppress.hierarchy)
      _FPRINTFV(f,"[ALBUM] %s\n",bg_tree_in_basename(tree));

    break;
  case BG_TREE_TYPE_ROOT:
    if (!tree->param->suppress.hierarchy)
      _FPRINTF(f,"[COLLECTION]\n");

    break;
  default:
    _DMESSAGEV("tree type %d unexpected",tree->vmt->type);
    goto e_type;
  }

  /////////////////////////////////////////////////////////////////////////////
  fflush(f);
#if defined (BG_PARAM_QUIET) // [
success:
#endif // ]
  return 0;
e_type:
  return -1;
}

static int bg_print_classic_tail(bg_tree_t *tree, int depth FFUNUSED, FILE *f)
{
  int suppress_hierarchy=tree->param->suppress.hierarchy;
  // path 1:  determine the maximum length of the involved labels.
  int width=bg_print_classic_width(tree);
  // path 2:  print out the results aligned according to the maximum length
  //   of the involved labels.
  bg_flags_agg_t agg;
  bg_print_conf_t *c;

#if defined (BG_PARAM_QUIET) // [
  if (tree->param->quiet)
    goto success;
#endif // ]

  if (width<=0) {
    _DWARNING("width");
    goto e_width;
  }

  if (!suppress_hierarchy||BG_TREE_TYPE_TRACK==tree->vmt->type) {
    for (agg=1,c=bg_print_conf;agg<BG_FLAGS_AGG_MAX;agg<<=1,++c) {
      if (!(agg&tree->param->flags.aggregate)) {
        // the aggregation isn't involved.
        continue;
      }
      else if (agg!=c->aggregate) {
        // wrong order.
        _DWARNING("aggregate mismatch");
        continue;
      }
      else
        bg_print_conf_tail(c,tree,width,f);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  fflush(f);
#if defined (BG_PARAM_QUIET) // [
success:
#endif // ]
  return 0;
e_width:
  return -1;
}

bg_print_vmt_t bg_print_classic_vmt={
  .id="classic",
  .infix=0,
  .encoding=bg_print_classic_encoding,
  .head=bg_print_classic_head,
  .tail=bg_print_classic_tail,
};
