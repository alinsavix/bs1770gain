/*
 * bg_print_csv.c
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

#define BG_PRINT_ARGV_HEADER
static int _bg_print_csv_tail(bg_tree_t *tree, int depth FFUNUSED, FILE *f,
    int header);

static void bg_print_conf_tail(bg_print_conf_t *c, bg_tree_t *tree FFUNUSED,
    FILE *f FFUNUSED, int header FFUNUSED)
{
  bg_param_t *param=tree->param;

  switch (c->argc) {
  case 1:
    if (c->argv[0].fn) {
      if (header) {
#if defined (_WIN32) // [
        if (stdout!=f&&stderr!=f) {
          fwprintf(f,L"%c",param->csv.separator);
          fputws(c->w.label.csv,f);
          fputws(L" (",f);
          bg_print_conf_unitw(f,1,c,tree->param,0);
          fputwc(L')',f);
        }
        else {
#endif // ]
          fprintf(f,"%c",param->csv.separator);
          fputs(c->n.label.csv,f);
          fputs(" (",f);
          bg_print_conf_unit(f,1,c,tree->param,0);
          fputc(')',f);
#if defined (_WIN32) // [
        }
#endif // ]
      }
      else {
#if defined (_WIN32) // [
        if (stdout!=f&&stderr!=f) {
          fwprintf(f,L"%c",param->csv.separator);
          fwprintf(f,c->argv[0].w.format,c->argv[0].fn(tree));
        }
        else {
#endif // ]
          fprintf(f,"%c",param->csv.separator);
          fprintf(f,c->argv[0].n.format,c->argv[0].fn(tree));
#if defined (_WIN32) // [
        }
#endif // ]
      }
    }
    else
      _DWARNING("argv[0]");

    break;
  case 2:
    if (c->argv[0].fn&&c->argv[1].fn) {
      if (header) {
#if defined (_WIN32) // [
        if (stdout!=f&&stderr!=f) {
          fwprintf(f,L"%c",param->csv.separator);
          fputws(c->w.label.csv,f);
          fputws(L" (",f);
          bg_print_conf_unitw(f,1,c,tree->param,0);
          fputwc(L')',f);
          fwprintf(f,L"%c",param->csv.separator);
          fputws(c->w.label.csv,f);
          fputws(L" (",f);
          bg_print_conf_unitw(f,1,c,tree->param,1);
          fputwc(L')',f);
        }
        else {
#endif // ]
          fprintf(f,"%c",param->csv.separator);
          fputs(c->n.label.csv,f);
          fputs(" (",f);
          bg_print_conf_unit(f,1,c,tree->param,0);
          fputc(')',f);
          fprintf(f,"%c",param->csv.separator);
          fputs(c->n.label.csv,f);
          fputs(" (",f);
          bg_print_conf_unit(f,1,c,tree->param,1);
          fputc(')',f);
#if defined (_WIN32) // [
        }
#endif // ]
      }
      else {
#if defined (_WIN32) // [
        if (stdout!=f&&stderr!=f) {
          fwprintf(f,L"%c",param->csv.separator);
          fwprintf(f,c->argv[0].w.format,c->argv[0].fn(tree));
          fwprintf(f,L"%c",param->csv.separator);
          fwprintf(f,c->argv[1].w.format,c->argv[1].fn(tree));
        }
        else {
#endif // ]
          fprintf(f,"%c",param->csv.separator);
          fprintf(f,c->argv[0].n.format,c->argv[0].fn(tree));
          fprintf(f,"%c",param->csv.separator);
          fprintf(f,c->argv[1].n.format,c->argv[1].fn(tree));
#if defined (_WIN32) // [
        }
#endif // ]
      }
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
static void bg_print_csv_encoding(bg_param_t *param, int bits)
{
  param->result.bits=bits;
}

static int _bg_print_csv_head(bg_tree_t *tree, int depth FFUNUSED, FILE *f)
{
  bg_param_t *param=tree->param;
  bg_track_t *track;

#if defined (BG_PARAM_QUIET) // [
  if (param->quiet)
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
      fwprintf(f,L"%.02f%c%lu%c%lu%c%S",param->norm,param->csv.separator,
          param->count.max,param->csv.separator,track->root.id,
          param->csv.separator,tree->source.basename);
    }
    else {
#endif // ]
      fprintf(f,"%.02f%c%lu%c%lu%c%s",param->norm,param->csv.separator,
          param->count.max,param->csv.separator,track->root.id,
          param->csv.separator,bg_tree_in_basename(tree));
#if defined (_WIN32) // [
    }
#endif // ]

    break;
  case BG_TREE_TYPE_ALBUM:
    if (!param->suppress.hierarchy) {
#if defined (_WIN32) // [
      if (stdout!=f&&stderr!=f) {
        fwprintf(f,L"%.02f%c%c%cALBUM %S",param->norm,param->csv.separator,
            param->csv.separator,param->csv.separator,tree->source.basename);
      }
      else {
#endif // ]
        fprintf(f,"%.02f%c%c%cALBUM %s",param->norm,param->csv.separator,
            param->csv.separator,param->csv.separator,
            bg_tree_in_basename(tree));
#if defined (_WIN32) // [
      }
#endif // ]
    }

    break;
  case BG_TREE_TYPE_ROOT:
    if (!param->suppress.hierarchy) {
#if defined (_WIN32) // [
      if (stdout!=f&&stderr!=f) {
        fwprintf(f,L"%.02f%c%c%cCOLLECTION",param->norm,param->csv.separator,
            param->csv.separator,param->csv.separator);
      }
      else {
#endif // ]
        fprintf(f,"%.02f%c%c%cCOLLECTION",param->norm,param->csv.separator,
            param->csv.separator,param->csv.separator);
#if defined (_WIN32) // [
      }
#endif // }
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

static int bg_print_csv_head(bg_tree_t *tree, int depth, FILE *f)
{
  bg_param_t *param=tree->param;

  if (stdout!=f&&stderr!=f) {
    if (param->csv.header.file)
      goto e_header;

    param->csv.header.file=1;
  }
  else {
    if (param->csv.header.console)
      goto e_header;

    param->csv.header.console=1;
  }

#if defined (_WIN32) // [
  if (stdout!=f&&stderr!=f) {
    fwprintf(f,L"norm%ctotal%cnumber%cfile",param->csv.separator,
        param->csv.separator,param->csv.separator);
    _bg_print_csv_tail(tree,0,f,1);
    fflush(f);
  }
  else {
#endif // ]
    fprintf(f,"norm%ctotal%cnumber%cfile",param->csv.separator,
        param->csv.separator,param->csv.separator);
    _bg_print_csv_tail(tree,0,f,1);
    fflush(f);
#if defined (_WIN32) // [
  }
#endif // ]
e_header:
  return _bg_print_csv_head(tree,depth,f);
}

static int _bg_print_csv_tail(bg_tree_t *tree, int depth FFUNUSED, FILE *f,
    int header)
{
  int suppress_hierarchy=tree->param->suppress.hierarchy;
  // path 2:  print out the results aligned according to the maximum length
  //   of the involved labels.
  bg_flags_agg_t agg;
  bg_print_conf_t *c;

#if defined (BG_PARAM_QUIET) // [
  if (tree->param->quiet)
    goto success;
#endif // ]

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
        bg_print_conf_tail(c,tree,f,header);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
  if (stdout!=f&&stderr!=f)
    fputwc(L'\n',f);
  else
#endif // ]
    fputc('\n',f);

  fflush(f);
#if defined (BG_PARAM_QUIET) // [
success:
#endif // ]
  return 0;
}

static int bg_print_csv_tail(bg_tree_t *tree, int depth FFUNUSED, FILE *f)
{
  return _bg_print_csv_tail(tree,depth,f,0);
}

bg_print_vmt_t bg_print_csv_vmt={
  .id="classic",
  .infix=0,
  .encoding=bg_print_csv_encoding,
  .head=bg_print_csv_head,
  .tail=bg_print_csv_tail,
};
