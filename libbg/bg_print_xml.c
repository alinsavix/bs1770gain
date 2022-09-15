/*
 * bs1770gain_print_xml.c
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

#if ! defined (_WIN32) // [
#include <ctype.h>
#endif // ]

static void bg_print_xml_encoding(bg_param_t *param, int bits)
{
  FILE *f=param->result.f;

  param->result.bits=bits;

#if defined (BG_PARAM_QUIET) // [
  if (param->quiet)
    goto success;
#endif // ]

#if defined (_WIN32) // [
  if (stdout!=f&&stderr!=f) {
    fwprintf(f,L"<?xml version=\"1.0\" encoding=\"UTF-%d\""
        " standalone=\"no\"?>\n",bits);
  }
  else {
#endif // ]
    fprintf(f,"<?xml version=\"1.0\" encoding=\"UTF-%d\""
        " standalone=\"no\"?>\n",bits);
#if defined (_WIN32) // [
  }
#endif // ]

#if defined (BG_PARAM_QUIET) // [
success:
  return;
#endif // ]
}

#if defined (_WIN32) // [
static void bg_print_xml_indentw(int depth, FILE *f)
{
  --depth;

   if (stdout==f||stderr==f) {
      _DMESSAGE("illegal attempt to write to the console");
      exit(1);
   }

   while (0<depth) {
     --depth;
     fputwc(L' ',f);
     fputwc(L' ',f);
   }
}
#endif // ]

static void bg_print_xml_indent(int depth, FILE *f)
{
  --depth;

   while (0<depth) {
     --depth;
     fputc(' ',f);
     fputc(' ',f);
   }
}

static int bg_print_xml_head(bg_tree_t *tree, int depth, FILE *f)
{
  bg_param_t *param=tree->param;
  bg_track_t *track=&tree->track;
  
#if defined (BG_PARAM_QUIET) // [
  if (tree->param->quiet)
    goto success;
#endif // ]

  switch (tree->vmt->type) {
  case BG_TREE_TYPE_TRACK:
#if defined (_WIN32) // [
    if (stdout!=f&&stderr!=f) {
      bg_print_xml_indentw(depth,f);

#if defined (BG_PARAM_XML_CDATA) // [
      if (param->xml.cdata) {
        fwprintf(f,L"<track total=\"%lu\" number=\"%lu\">\n",
            param->count.max);
        bg_print_xml_indentw(depth+1,f);
        fwprintf(f,L"<file><![CDATA[%S]]></file>\n",tree->source.basename);
      }
      else {
#endif // ]
        fwprintf(f,L"<track total=\"%lu\" number=\"%lu\" file=\"%S\">\n",
            param->count.max,track->root.id,tree->source.basename);
#if defined (BG_PARAM_XML_CDATA) // [
      }
#endif // ]
    }
    else {
#endif // ]
#if defined (BG_PARAM_XML_CDATA) // [
      bg_print_xml_indent(depth,f);

      if (param->xml.cdata) {
        fprintf(f,"<track total=\"%lu\" number=\"%lu\">\n",
            param->count.max,track->root.id);
        bg_print_xml_indent(depth+1,f);
        fprintf(f,"<file><![CDATA[%s]]></file>\n",bg_tree_in_basename(tree));
      }
      else {
#endif // ]
        fprintf(f,"<track total=\"%lu\" number=\"%lu\" file=\"%s\">\n",
            param->count.max,track->root.id,bg_tree_in_basename(tree));
#if defined (BG_PARAM_XML_CDATA) // [
      }
#endif // ]
#if defined (_WIN32) // [
    }
#endif // ]

    break;
  case BG_TREE_TYPE_ALBUM:
#if 0 // [
    if (!param->suppress.hierarchy) {
#if defined (_WIN32) // [
      if (stdout!=f&&stderr!=f) {
        bg_print_xml_indentw(depth,f);
        fwprintf(f,L"<album folder=\"%S\">\n",tree->source.basename);
      }
      else {
#endif // ]
        bg_print_xml_indent(depth,f);
        fprintf(f,"<album folder=\"%s\">\n",bg_tree_in_basename(tree));
#if defined (_WIN32) // [
      }
#endif // ]
    }
#else // ] [
    if (!param->suppress.hierarchy) {
#if defined (_WIN32) // [
    if (stdout!=f&&stderr!=f) {
      bg_print_xml_indentw(depth,f);

#if defined (BG_PARAM_XML_CDATA) // [
      if (param->xml.cdata) {
        fwprintf(f,L"<album>\n");
        bg_print_xml_indentw(depth+1,f);
        fwprintf(f,L"<folder><![CDATA[%S]]></folder>\n",tree->source.basename);
      }
      else {
#endif // ]
        fwprintf(f,L"<album folder=\"%S\">\n",tree->source.basename);
#if defined (BG_PARAM_XML_CDATA) // [
      }
#endif // ]
    }
    else {
#endif // ]
#if defined (BG_PARAM_XML_CDATA) // [
      bg_print_xml_indent(depth,f);

      if (param->xml.cdata) {
        fprintf(f,"<album>\n");
        bg_print_xml_indent(depth+1,f);
        fprintf(f,"<folder><![CDATA[%s]]></folder>\n",
						bg_tree_in_basename(tree));
      }
      else {
#endif // ]
        fprintf(f,"<album file=\"%s\">\n",bg_tree_in_basename(tree));
#if defined (BG_PARAM_XML_CDATA) // [
      }
#endif // ]
#if defined (_WIN32) // [
    }
#endif // ]
		}
#endif // ]

    break;
  case BG_TREE_TYPE_ROOT:

#if defined (_WIN32) // [
    if (stdout!=f&&stderr!=f) {
      bg_print_xml_indentw(depth,f);
      fwprintf(f,L"<bs1770gain norm=\"%0.2f\">\n",param->norm);
    }
    else {
#endif // ]
      bg_print_xml_indent(depth,f);
      fprintf(f,"<bs1770gain norm=\"%0.2f\">\n",param->norm);
#if defined (_WIN32) // [
    }
#endif // ]

    break;
  default:
    _DMESSAGE("unknown type");
    goto e_type;
  }

  /////////////////////////////////////////////////////////////////////////////
  fflush(f);
#if defined (BG_PARAM_QUIET) // [
success:
#endif // ]
  return 0;
//cleanup:
e_type:
  return -1;
}

static void bg_print_conf_tail(bg_print_conf_t *c, bg_tree_t *tree,
    int depth, FILE *f)
{

  switch (c->argc) {
  case 1:
#if defined (_WIN32) // [
    if (stdout!=f&&stderr!=f) {
      bg_print_xml_indentw(depth,f);
      fwprintf(f,L"<%S ",c->w.label.xml);
      bg_print_conf_unitw(f,1,c,tree->param,0);
      fwprintf(f,L"=\"");
      fwprintf(f,c->argv[0].w.format,c->argv[0].fn(tree));
#if defined (BG_SERGEY_XML_FIX) // [
      fwprintf(f,L"\"/>\n");
#endif // ]
    }
    else {
#endif // ]
      bg_print_xml_indent(depth,f);
      fprintf(f,"<%s ",c->n.label.xml);
      bg_print_conf_unit(f,1,c,tree->param,0);
      fprintf(f,"=\"");
      fprintf(f,c->argv[0].n.format,c->argv[0].fn(tree));
#if defined (BG_SERGEY_XML_FIX) // [
      fprintf(f,"\"/>\n");
#endif // ]
#if defined (_WIN32) // [
    }
#endif // ]

    break;
  case 2:
#if defined (_WIN32) // [
    if (stdout!=f&&stderr!=f) {
      bg_print_xml_indentw(depth,f);
      fwprintf(f,L"<%S ",c->w.label.xml);
      bg_print_conf_unitw(f,1,c,tree->param,0);
      fwprintf(f,L"=\"");
      fwprintf(f,c->argv[0].w.format,c->argv[0].fn(tree));
      fwprintf(f,L"\" ");
      bg_print_conf_unitw(f,1,c,tree->param,1);
      fwprintf(f,L"=\"");
      fwprintf(f,c->argv[1].w.format,c->argv[1].fn(tree));
      fwprintf(f,L"\"/>\n");
    }
    else {
#endif // ]
      bg_print_xml_indent(depth,f);
      fprintf(f,"<%s ",c->n.label.xml);
      bg_print_conf_unit(f,1,c,tree->param,0);
      fprintf(f,"=\"");
      fprintf(f,c->argv[0].n.format,c->argv[0].fn(tree));
      fprintf(f,"\" ");
      bg_print_conf_unit(f,1,c,tree->param,1);
      fprintf(f,"=\"");
      fprintf(f,c->argv[1].n.format,c->argv[1].fn(tree));
      fprintf(f,"\"/>\n");
#if defined (_WIN32) // [
    }
#endif // ]

    break;
  default:
    _DWARNINGV("argc:%d",c->argc);
    break;
  }
}

static int bg_print_xml_tail(bg_tree_t *tree, int depth, FILE *f)
{
  bg_param_t *param=tree->param;
  bg_flags_agg_t agg;
  bg_print_conf_t *c;

#if defined (BG_PARAM_QUIET) // [
  if (param->quiet)
    goto success;
#endif // ]

  if (BG_TREE_TYPE_TRACK==tree->vmt->type||!param->suppress.hierarchy) {
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
      else
        bg_print_conf_tail(c,tree,depth+1,f);
    }
  }

  switch (tree->vmt->type) {
  case BG_TREE_TYPE_TRACK:

#if defined (_WIN32) // [
    if (stdout!=f&&stderr!=f) {
      bg_print_xml_indentw(depth,f);
      fwprintf(f,L"</track>\n");
    }
    else {
#endif // ]
      bg_print_xml_indent(depth,f);
      fprintf(f,"</track>\n");
#if defined (_WIN32) // [
    }
#endif // ]

    break;
  case BG_TREE_TYPE_ALBUM:
    if (!param->suppress.hierarchy) {
#if defined (_WIN32) // [
      if (stdout!=f&&stderr!=f) {
        bg_print_xml_indentw(depth,f);
        fwprintf(f,L"</album>\n");
      }
      else {
#endif // ]
        bg_print_xml_indent(depth,f);
        fprintf(f,"</album>\n");
      }
#if defined (_WIN32) // [
    }
#endif // ]

    break;
  case BG_TREE_TYPE_ROOT:

#if defined (_WIN32) // [
    if (stdout!=f&&stderr!=f) {
      bg_print_xml_indentw(depth,f);
      fwprintf(f,L"</bs1770gain>\n");
    }
    else {
#endif // ]
      bg_print_xml_indent(depth,f);
      fprintf(f,"</bs1770gain>\n");
#if defined (_WIN32) // [
    }
#endif // ]

    break;
  default:
    _DMESSAGE("unknown type");
    goto e_type;
  }

  /////////////////////////////////////////////////////////////////////////////
  fflush(f);
#if defined (BG_PARAM_QUIET) // [
success:
#endif // ]
  return 0;
//cleanup:
e_type:
  return -1;
}

bg_print_vmt_t bg_print_xml_vmt={
  .id="xml",
  .infix=1,
  .encoding=bg_print_xml_encoding,
  .head=bg_print_xml_head,
  .tail=bg_print_xml_tail,
};
