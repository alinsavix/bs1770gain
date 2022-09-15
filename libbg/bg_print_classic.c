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

static void bg_print_classic_indent(FILE *f, int width)
{
  enum { SIZE=128 };
  
  union {
#if defined (_WIN32) // [
    // wide string representation.
    wchar_t w[SIZE];
#endif // ]
    // narrow string representation.
    char n[SIZE];
  } format;

  width+=2;

  // indentation.
#if defined (_WIN32) // [
  if (stdout!=f&&stderr!=f) {
    _snwprintf(format.w,SIZE,L"%%%ds",width);
    fwprintf(f,format.w,L"");
  }
  else {
#endif // ]
    snprintf(format.n,SIZE,"%%%ds",width);
    fprintf(f,format.n,"");
#if defined (_WIN32) // [
  }
#endif // ]
}

static void bg_print_conf_tail(bg_print_conf_t *c, bg_tree_t *tree FFUNUSED,
    int width FFUNUSED, FILE *f FFUNUSED)
{
  switch (c->argc) {
  case 1:
    if (c->argv[0].fn) {
#if defined (_WIN32) // [
      if (stdout!=f&&stderr!=f) {
        bg_print_classic_indent(f,width-wcslen(c->w.label.classic));
        fwprintf(f,L"%S: ",c->w.label.classic);
        fwprintf(f,c->argv[0].w.format,c->argv[0].fn(tree));
        fputwc(L' ',f);
        bg_print_conf_unitw(f,0,c,tree->param,0);
        fputwc(L'\n',f);
      }
      else {
#endif // ]
        bg_print_classic_indent(f,width-strlen(c->n.label.classic));
        fprintf(f,"%s: ",c->n.label.classic);
        fprintf(f,c->argv[0].n.format,c->argv[0].fn(tree));
        fputc(' ',f);
        bg_print_conf_unit(f,0,c,tree->param,0);
        fputc('\n',f);
#if defined (_WIN32) // [
      }
#endif // ]
    }
    else
      _DWARNING("argv[0]");

    break;
  case 2:
    if (c->argv[0].fn&&c->argv[1].fn) {
#if defined (_WIN32) // [
      if (stdout!=f&&stderr!=f) {
        bg_print_classic_indent(f,width-wcslen(c->w.label.classic));
        fwprintf(f,L"%S: ",c->w.label);
        fwprintf(f,c->argv[0].w.format,c->argv[0].fn(tree));
        fputwc(L' ',f);
        bg_print_conf_unitw(f,0,c,tree->param,0);
        fwprintf(f,L" / ");
        fwprintf(f,c->argv[1].w.format,c->argv[1].fn(tree));
        fputwc(L' ',f);
        bg_print_conf_unitw(f,0,c,tree->param,1);
        fputwc(L'\n',f);
      }
      else {
#endif // ]
        bg_print_classic_indent(f,width-strlen(c->n.label.classic));
        fprintf(f,"%s: ",c->n.label.classic);
        fprintf(f,c->argv[0].n.format,c->argv[0].fn(tree));
        fputc(' ',f);
        bg_print_conf_unit(f,0,c,tree->param,0);
        fprintf(f," / ");
        fprintf(f,c->argv[1].n.format,c->argv[1].fn(tree));
        fputc(' ',f);
        bg_print_conf_unit(f,0,c,tree->param,1);
        fputc('\n',f);
#if defined (_WIN32) // [
      }
#endif // ]
      fflush(f);
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

static int bg_print_conf_len(bg_print_conf_t *c, FILE *f FFUNUSED)
{
#if defined (_WIN32) // [
  if (stdout!=f&&stderr!=f)
    return wcslen(c->w.label.classic);
  else
#endif // ]
    return strlen(c->n.label.classic);
}

static int bg_print_classic_width(bg_tree_t *tree, FILE *f)
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
    else if (agg!=c->agg) {
      // wrong order.
      _DWARNING("aggregate mismatch");
      continue;
    }
    else if ((len=bg_print_conf_len(c,f))<0) {
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

#if defined (_WIN32) // [
    if (stdout!=f&&stderr!=f) {
      fwprintf(f,L"[%lu/%lu] %S\n",track->root.id,tree->param->count.max,
          tree->source.basename);
    }
    else {
#endif // ]
      fprintf(f,"[%lu/%lu] %s\n",track->root.id,tree->param->count.max,
          bg_tree_in_basename(tree));
#if defined (_WIN32) // [
    }
#endif // ]

    break;
  case BG_TREE_TYPE_ALBUM:
    if (!tree->param->suppress.hierarchy) {
#if defined (_WIN32) // [
      if (stdout!=f&&stderr!=f)
        fwprintf(f,L"[ALBUM] %S\n",tree->source.basename);
      else
#endif // ]
        fprintf(f,"[ALBUM] %s\n",bg_tree_in_basename(tree));
    }

    break;
  case BG_TREE_TYPE_ROOT:
    if (!tree->param->suppress.hierarchy) {
#if defined (_WIN32) // [
      if (stdout!=f&&stderr!=f)
        fwprintf(f,L"[COLLECTION]\n");
      else
#endif // ]
        fprintf(f,"[COLLECTION]\n");
    }

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
  int width=bg_print_classic_width(tree,f);
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
      else if (agg!=c->agg) {
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
