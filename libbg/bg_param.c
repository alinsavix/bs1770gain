/*
 * bg_param.c
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
static bg_pilot_callback_t bg_pilot_callback;

///////////////////////////////////////////////////////////////////////////////
static bg_param_unit_t bg_param_unit_ebu={
  .n.lu="LU",
#if defined (BG_UNIT_LRA) // [
  .n.lra="LRA",
#else // ] [
  .n.lra="LU",
#endif // ]
  .n.sp="SP",
  .n.tp="TP",
#if defined (_WIN32) // [
  .w.lu=L"LU",
#if defined (BG_UNIT_LRA) // [
  .w.lra=L"LRA",
#else // ] [
  .w.lra=L"LU",
#endif // ]
  .w.sp=L"SP",
  .w.tp=L"TP",
#endif // ]
};

static bg_param_unit_t bg_param_unit_db={
  .n.lu="dB",
  .n.lra="dB",
  .n.sp="dB",
  .n.tp="dB",
#if defined (_WIN32) // [
  .w.lu=L"dB",
  .w.lra=L"dB",
  .w.sp=L"dB",
  .w.tp=L"dB",
#endif // ]
};

///////////////////////////////////////////////////////////////////////////////
int bg_param_create(bg_param_t *param)
{
  memset(param,0,sizeof *param);
  param->count.cur=0;
  param->count.max=0;
  param->process=0;
#if defined (BG_PARAM_SKIP_SCAN) // [
  param->skip_scan=0;
#endif // ]
#if defined (BG_PARAM_SCRIPT) // [
  param->script=NULL;
#endif // ]
#if defined (BG_SAMPLES_COUNT) // [
  param->upsampler.threshould=0;
#endif // ]

  param->csv.separator=FFL('\t');
#if defined (BG_PARAM_XML_CDATA) // [
  param->xml.cdata=0;
#endif // ]

#if defined (BG_PARAM_SHELL) // [
#if defined (_WIN32) // [
  param->shell.interpreter=NULL;
  param->shell.parameter=L"-c";
#else // ] [
  param->shell.interpreter="/bin/sh";
  param->shell.parameter="-c";
#endif // ]
#endif // ]

#if defined (FF_INPUT_LIST) // [
  param->list.in=0;
  param->list.out=0;
#endif // ]

  if (bg_pilot_create(&param->pilot,20,&bg_pilot_callback,param)<0) {
    _DMESSAGE("creating pilot");
    goto e_pilot;
  }

  if (bg_analyzer_create(&param->analyzer)<0) {
    _DMESSAGE("creating analyzer");
    goto e_analyzer;
  }

  param->print.vmt=&bg_print_classic_vmt;

#if defined (FF_PROGRESS_STDERR) // [
  if (ff_printer_create(&param->printer,stderr)<0) {
    _DMESSAGE("creating printer");
    goto e_printer;
  }
#else // ] [
  if (ff_printer_create(&param->printer,stdout)<0) {
    _DMESSAGE("creating printer");
    goto e_printer;
  }
#endif // ]

  param->loglevel=AV_LOG_QUIET;
#if defined (_WIN32) // [
  param->codec.name[0]=L'\0';
#else // ] [
  param->codec.name=NULL;
#endif // ]
  param->temp_prefix=BG_TEMP_PREFIX;
  param->suppress.hierarchy=0;
  param->suppress.progress=0;
#if defined (_WIN32) // [
  // if LANG is set to e.g. "en_US.UTF-8" we assume we're run from
  // e.g. MSYS2 shell undestanding UTF-8 otherwise from MS console using
  // codepage OEM. In the latter case we need an OEM representation of
  // e.g. basename.
  param->oem=0;
#endif // ]
  param->decode.request.sample_fmt=-1;
  param->decode.request.channel_layout=-1ll;
  param->decode.drc.enabled=0;
  param->decode.drc.scale=0.0;
  param->result.f=stdout;
  param->output.dirname=NULL;
  param->ai=-1;
  param->vi=-1;
  param->flags.extension=0;
  param->flags.mode=0;
  param->flags.aggregate=BG_FLAGS_AGG_INTEGRATED;
  param->flags.norm=BG_FLAGS_NORM_EBU;
#if defined (BG_PARAM_DUMP) // [
  param->dump=0;
#endif // ]
#if defined (BG_PARAM_STEREO) // [
  param->stereo=0;
#else // ] [
  param->decode.request.channel_layout=-1ll;
#endif // ]
  param->norm=-23.0;
  param->preamp=0.0;
  param->weight.enabled=0;
  param->weight.value=1.0;
#if defined (BG_CLOCK) // [
  param->time=0;
#endif // ]
#if defined (BG_PARAM_LFE) // [
#if defined (LIB1770_LFE) // [
  param->lfe=LIB1770_LFE;
#elif defined (BG_CHANNEL_LFE) // ] [
  param->lfe=BG_CHANNEL_LFE;
#else // ] [
  param->lfe=-1;
#endif // ]
#endif // ]
#if defined (_WIN32) && defined (BG_WIN32_CREATE_LOCALE) // [
  param->locale=0;
#endif // ]
  param->overwrite=0;
  param->unit=&bg_param_unit_ebu;
  strncpy(param->tag.pfx,"REPLAYGAIN_",(sizeof param->tag)-1);
  param->out.sfx=NULL;
  param->ext.audio=FFL("flac");
  param->ext.video=FFL("mkv");
  param->interval.begin=-1ll;
  param->interval.duration=-1ll;
#if defined (BG_PARAM_QUIET) // [
  param->quiet=0;
#endif // ]
#if defined (BG_PARAM_THREADS) // [
  param->nthreads=0;
#endif // ]
#if defined (BG_PARAM_SLEEP) // [
  param->sleep=0;
#endif // ]
  ////////
  param->momentary.ms=400.0;
  param->momentary.partition=4;
  param->momentary.mean.gate=-10.0;
  param->momentary.range.gate=-20.0;
  param->momentary.range.lower_bound=0.1;
  param->momentary.range.upper_bound=0.95;
  ////////
  param->shortterm.ms=3000.0;
  param->shortterm.partition=3;
  param->shortterm.mean.gate=-10.0;
  param->shortterm.range.gate=-20.0;
  param->shortterm.range.lower_bound=0.1;
  param->shortterm.range.upper_bound=0.95;

#if defined (FF_PROGRESS_STDERR) // [
  param->stdprog=stderr;
#endif // ]
  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  ff_printer_destroy(&param->printer);
e_printer:
  if (param->analyzer.vmt)
    param->analyzer.vmt->destroy(&param->analyzer);
e_analyzer:
  bg_pilot_destroy(&param->pilot);
e_pilot:
  return -1;
}

void bg_param_destroy(bg_param_t *param)
{
  ff_printer_destroy(&param->printer);

  if (param->analyzer.vmt)
    param->analyzer.vmt->destroy(&param->analyzer);

  bg_pilot_destroy(&param->pilot);
}

int bg_param_alloc_arguments(bg_param_t *param, size_t size)
{
  /////////////////////////////////////////////////////////////////////////////
  if (!size) {
    _DMESSAGE("nothing to analyze");
    goto e_size;
  }

  /////////////////////////////////////////////////////////////////////////////
  param->argv.min=malloc(size*sizeof *param->argv.min);

  if (!param->argv.min) {
    _DMESSAGE("allocating");
    goto e_alloc;
  }

  memset(param->argv.min,0,size*sizeof *param->argv.min);
  param->argv.cur=param->argv.min;
  param->argv.max=param->argv.min+size;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  free(param->argv.min);
e_alloc:
e_size:
  return -1;
}

void bg_param_free_argumets(bg_param_t *param)
{
  free(param->argv.min);
}

void bg_param_set_unit_ebu(bg_param_t *param)
{
  param->unit=&bg_param_unit_ebu;
}

void bg_param_set_unit_db(bg_param_t *param)
{
  param->unit=&bg_param_unit_db;
}

void bg_param_set_process(bg_param_t *param)
{
  ff_printer_clear(&param->printer);
  param->process=1;
  param->count.max=param->count.cur;
  param->count.cur=0u;
}

//#define BG_PARAM_LOOP_INNER
#if defined (BG_PARAM_LOOP_INNER) // [
static int bg_param_loop_inner(bg_param_t *param, ffchar_t const *argv)
{
  int err=-1;

  /////////////////////////////////////////////////////////////////////////////
  if (bg_root_create(&param->root,param)<0) {
    _DMESSAGE("creating root");
    goto e_root;
  }

  param->tos=&param->root;

  if (param->process) {
    ///////////////////////////////////////////////////////////////////////////
    if (bg_analyzer_album_prefix(&param->analyzer,&param->root)<0) {
      _DMESSAGE("prefix");
      goto e_prefix;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_pilot_loop(&param->pilot,argv)<0) {
    _DMESSAGE("looping");
    goto e_loop;
  }

  if (param->process) {
    ///////////////////////////////////////////////////////////////////////////
    if (bg_analyzer_album_suffix(&param->analyzer,&param->root)<0) {
      _DMESSAGE("suffix");
      goto e_suffix;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
e_suffix:
e_loop:
e_prefix:
  param->root.vmt->destroy(&param->root);
e_root:
  return err;
}
#endif // ]

int bg_param_loop(bg_param_t *param, ffchar_t *const *argv)
{
  int err=-1;

#if ! defined (BG_PARAM_LOOP_INNER) // [
  /////////////////////////////////////////////////////////////////////////////
  if (bg_root_create(&param->root,param)<0) {
    _DMESSAGE("creating root");
    goto e_root;
  }

  param->tos=&param->root;

  if (param->process) {
    ///////////////////////////////////////////////////////////////////////////
    if (bg_analyzer_album_prefix(&param->analyzer,&param->root)<0) {
      _DMESSAGE("prefix");
      goto e_prefix;
    }
  }
#endif // ]

  for (param->argv.cur=param->argv.min;param->argv.cur<param->argv.max;
      ++param->argv.cur) {
#if defined (BG_PARAM_LOOP_INNER) // [
    ///////////////////////////////////////////////////////////////////////////
    if (bg_param_loop_inner(param,*argv++)<0) {
        _DMESSAGE("inner loop");
        goto e_inner;
    }
#else // ] [
    ///////////////////////////////////////////////////////////////////////////
    if (bg_pilot_loop(&param->pilot,*argv++)<0) {
      _DMESSAGE("looping");
      goto e_loop;
    }
#endif // ]
  }

#if ! defined (BG_PARAM_LOOP_INNER) // [
  if (param->process) {
    ///////////////////////////////////////////////////////////////////////////
    if (bg_analyzer_album_suffix(&param->analyzer,&param->root)<0) {
      _DMESSAGE("suffix");
      goto e_suffix;
    }
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  err=0;
//cleanup:
#if defined (BG_PARAM_LOOP_INNER) // [
e_inner:
#else // ] [
e_suffix:
e_loop:
e_prefix:
  param->root.vmt->destroy(&param->root);
e_root:
#endif // ]
  return err;
}

///////////////////////////////////////////////////////////////////////////////
static int bg_pilot_callback_leaf_enter(bg_pilot_hist_t *hist, void *data)
{
  bg_param_t *param=data;
  bg_tree_t *parent=param->tos;
  bg_tree_t *tree;

  /////////////////////////////////////////////////////////////////////////////
  if (bg_leaf_create(&tree,param,parent,hist->path)<0) {
#if defined (BG_TREE_CREATE_CHILD_WARNING) // [
    _DWARNING("creating leaf");
#endif // ]
    hist->data=NULL;
    goto success;
  }

  /////////////////////////////////////////////////////////////////////////////
  hist->data=tree;
  param->tos=tree;

  /////////////////////////////////////////////////////////////////////////////
success:
  return 0;
}

static void bg_pilot_callback_leaf_leave(bg_pilot_hist_t *hist, void *data)
{
  bg_param_t *param=data;
  bg_tree_t *tree=hist->data;

  if (tree) {
    // unlink.
    hist->data=NULL;
    param->tos=tree->parent;
  }
}

////////
static int bg_pilot_callback_branch_enter(bg_pilot_hist_t *hist, void *data)
{
  bg_param_t *param=data;
  bg_tree_t *parent=param->tos;
  bg_tree_t *tree;

  /////////////////////////////////////////////////////////////////////////////
  if (bg_album_create(&tree,param,parent,hist->path)<0) {
    _DMESSAGE("creating album");
    goto e_album;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (param->process&&bg_analyzer_album_prefix(&param->analyzer,tree)<0) {
    _DMESSAGE("analyzer prefix");
    goto e_prefix;
  }

  /////////////////////////////////////////////////////////////////////////////
  hist->data=tree;
  param->tos=tree;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_prefix:
  // just an indirect call to bg_album_destroy().
  tree->vmt->destroy(tree);
e_album:
  return -1;
}

static void bg_pilot_callback_branch_leave(bg_pilot_hist_t *hist, void *data)
{
  bg_param_t *param=data;
  bg_tree_t *tree=hist->data;

  /////////////////////////////////////////////////////////////////////////////
  if (tree) {
    if (param->process) {
      if (bg_analyzer_album_suffix(&param->analyzer,tree)<0) {
        _DMESSAGE("analyzing");
        goto e_analyze;
      }

      if (tree->parent&&bg_tree_merge(tree->parent,tree)<0) {
        _DMESSAGE("merging");
        goto e_merge;
      }
    }
  e_merge:
  e_analyze:
    // unlink.
    hist->data=NULL;
    param->tos=tree->parent;

    // just an indirect call to bg_album_destroy().
    tree->vmt->destroy(tree);
  }
}

static bg_pilot_callback_t bg_pilot_callback={
  .leaf.enter=bg_pilot_callback_leaf_enter,
  .leaf.leave=bg_pilot_callback_leaf_leave,
  .branch.enter=bg_pilot_callback_branch_enter,
  .branch.leave=bg_pilot_callback_branch_leave,
};
