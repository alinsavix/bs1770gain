/*
 * bg_track.c
 *
 * Copyright (C) 2019 Peter Belkner <pbelkner@users.sf.net>
 * Nanos gigantum humeris insidentes #TeamWhite
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
static bg_tree_vmt_t bg_track_vmt;
static ff_input_callback_t bg_input_callback;

int bg_track_content_create(bg_tree_t *tree)
{
  int err=-1;
  bg_param_t *param=tree->param;
  ff_printer_t *p=&param->printer;
  bg_track_t *track=&tree->track;

  /////////////////////////////////////////////////////////////////////////////
#if 0 // [
  // DON'T do this!!! bg_track_create() is called from bg_child_create()
  // which in front already has called bg_tree_common_create() and just
  // leaves us with setting the vmt!
  if (bg_tree_common_create(tree,param,param,path,&bg_track_vmt)<0) {
    DMESSAGE("creating tree");
    goto etree;
  }
#else // ] [
  tree->vmt=&bg_track_vmt;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  memset(&track->input,0,sizeof track->input);
  memset(&track->analyzer,0,sizeof track->analyzer);
  track->target.title=NULL;

  /////////////////////////////////////////////////////////////////////////////
#if defined (BG_TREE_CREATE_CHILD_WARNING) // [
  err=ff_input_create(&track->input,&bg_input_callback,tree,1);

  if (err<0) {
    FFVWARNING(FFL("opening track \"%s\""),pathw);
    goto e_input;
  }
#else // ] [
  err=ff_input_create(&track->input,&bg_input_callback,tree,0,p);

  if (err<0)
    goto e_input;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (!track->input.audio.ctx->channel_layout) {
    DMESSAGE("missing input channel layout");
    goto e_channel_layout;
  }

  /////////////////////////////////////////////////////////////////////////////
  track->root.id=++param->count.cur;

  /////////////////////////////////////////////////////////////////////////////
#if defined (BG_PARAM_QUIET) // [
  if (!param->quiet&&!param->process&&!param->suppress.progress) {
#else // ] [
  if (!param->process&&!param->suppress.progress) {
#endif // ]
    ff_printer_reset(p);

#if defined (_WIN32) // [
    if (stdout==p->f||stderr==p->f)
      ff_printer_wprintf(p,L"%d",track->root.id);
    else
#endif // ]
      ff_printer_printf(p,"%d",track->root.id);
#if ! defined (BG_PARAM_QUIET) // [
  }
#else // ] [
  }
#endif // ]

  ++tree->parent->album.nleafs;
  ff_input_close(&track->input);

  return 0;
//cleanup:
e_channel_layout:
  ff_input_destroy(&track->input);
e_input:
  return err;
}

///////////////////////////////////////////////////////////////////////////////
static void bg_track_destroy(bg_tree_t *tree)
{
  bg_param_t *param=tree->param;
  bg_track_t *track=&tree->track;

  if (tree->parent)
    bg_album_pop(tree->parent);

  if (!param->process)
    tree->argv->lift=tree->depth;

  track->input.cb.in->stats.destroy(tree);
  ff_input_destroy(&track->input);
  bg_tree_common_destroy(tree);
  free(tree);
}

static int bg_track_accept(bg_tree_t *tree, bg_visitor_t *vis)
{
  return vis->vmt->dispatch_track(vis,tree);
}

static bg_tree_vmt_t bg_track_vmt={
#if defined (PBU_DEBUG) // [
  .id=FFL("track"),
#endif // ]
  .type=BG_TREE_TYPE_TRACK,
  .destroy=bg_track_destroy,
  .accept=bg_track_accept,
  .annotation={
    .create=bg_track_annotation_create,
    .destroy=bg_track_annotation_destroy,
  },
};

///////////////////////////////////////////////////////////////////////////////
static const char *input_path(const void *data)
{
#if defined (_WIN32) // [
  return ((const bg_tree_t *)data)->utf8.path;
#else // ] [
  return ((const bg_tree_t *)data)->source.path;
#endif // ]
}

#if defined (_WIN32) // [
static const wchar_t *input_pathw(const void *data)
{
  return ((const bg_tree_t *)data)->source.path;
}
#endif // ]

static const ff_param_decode_t *input_decode(const void *data)
{
  return &((const bg_tree_t *)data)->param->decode;
}

static int input_upsample(const void *data)
{
  const bg_tree_t *tree=data;

  return BG_FLAGS_AGG_TRUEPEAK&tree->param->flags.aggregate?4:0;
}

static int input_transcode(const void *data)
{
  return BG_FLAGS_MODE_APPLY&((const bg_tree_t *)data)->param->flags.mode;
}

static int input_csv(const void *data)
{
  return BG_FLAGS_EXT_CSV&((const bg_tree_t *)data)->param->flags.extension;
}

static int input_suppress_progress(const void *data)
{
#if defined (BG_PARAM_QUIET) // [
  bg_param_t *param=((const bg_tree_t *)data)->param;

  return param->quiet||param->suppress.progress;
#else // ] [
  return ((const bg_tree_t *)data)->param->suppress.progress;
#endif // ]
}

static int64_t input_interval_begin(const void *data)
{
  const bg_param_t *param=((const bg_tree_t *)data)->param;

  return param->interval.begin;
}

static int64_t input_interval_duration(const void *data)
{
  const bg_param_t *param=((const bg_tree_t *)data)->param;

  return param->interval.duration;
}

static int input_stats_create(void *data, const AVCodecParameters *codecpar)
{
  bg_tree_t *tree=data;
  bg_track_t *track=&tree->track;
  int channels=codecpar->channels<FF_LFE_THRESHOLD
      ?codecpar->channels
      :codecpar->channels-1;

  /////////////////////////////////////////////////////////////////////////////
  if (tree->stats.momentary||tree->stats.shortterm) {
    track->filter.pre=lib1770_pre_new(codecpar->sample_rate,channels);

    if (!track->filter.pre) {
      DMESSAGE("creating pre-filter");
      goto epre;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  if (tree->stats.momentary) {
    track->block.momentary=lib1770_block_new(codecpar->sample_rate,
        tree->param->momentary.ms,tree->param->momentary.partition);

    if (!track->block.momentary) {
      DMESSAGE("creating momentary block");
      goto emomentary;
    }

    lib1770_block_add_stats(track->block.momentary,tree->stats.momentary);
    lib1770_pre_add_block(track->filter.pre,track->block.momentary);
  }
  else
    track->block.momentary=NULL;

  /////////////////////////////////////////////////////////////////////////////
  if (tree->stats.shortterm) {
    track->block.shortterm=lib1770_block_new(codecpar->sample_rate,
        tree->param->shortterm.ms,tree->param->shortterm.partition);

    if (!track->block.shortterm) {
      DMESSAGE("creating shortterm block");
      goto eshortterm;
    }

    lib1770_block_add_stats(track->block.shortterm,tree->stats.shortterm);
    lib1770_pre_add_block(track->filter.pre,track->block.shortterm);
  }
  else
    track->block.shortterm=NULL;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  if (track->block.shortterm)
    lib1770_block_close(track->block.shortterm);
eshortterm:
  if (track->block.momentary)
    lib1770_block_close(track->block.momentary);
emomentary:
  if (track->filter.pre)
    lib1770_pre_close(track->filter.pre);
epre:
  return -1;
}

static void input_stats_destroy(void *data)
{
  bg_track_t *track=&((bg_tree_t *)data)->track;

  if (track->block.shortterm)
    lib1770_block_close(track->block.shortterm);

  if (track->block.momentary)
    lib1770_block_close(track->block.momentary);

  if (track->filter.pre)
    lib1770_pre_close(track->filter.pre);
}

static int input_stats_add(void *data, int upsampled, AVFrame *frame)
{
  bg_tree_t *tree=data;
  bg_track_t *track=&tree->track;
  lib1770_sample_t sample={ 0 };
  ff_iter_t i={ 0 };

  if (frame) {
    for (ff_iter_first(&i,frame);i.vmt->valid(&i);i.vmt->next(&i)) {
//DWRITELN("{");
      if (upsampled) {
#if defined (FF_UPSAMPLE_MODIFYX) // [
        i.vmt->norm(&i,NULL,&tree->stats.truepeak,upsampled);
#else // ] [
        i.vmt->norm(&i,NULL,&tree->stats.truepeak);
#endif // ]
      }
      else if (track->filter.pre) {
//DWRITELN(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
#if defined (FF_UPSAMPLE_MODIFYX) // [
        i.vmt->norm(&i,sample,&tree->stats.samplepeak,upsampled);
#else // ] [
        i.vmt->norm(&i,sample,&tree->stats.samplepeak);
#endif // ]
//DWRITELN("================================================");
        lib1770_pre_add_sample(track->filter.pre,sample);
//DWRITELN("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
      }
      else {
#if defined (FF_UPSAMPLE_MODIFYX) // [
        i.vmt->norm(&i,NULL,&tree->stats.samplepeak,0);
#else // ] [
        i.vmt->norm(&i,NULL,&tree->stats.samplepeak);
#endif // ]
      }
//DWRITELN("}");
    }
  }
#if ! defined (FF_INPUT_CALLBACK_FLUSH) // [
#if defined (FF_UPSAMPLE_MODIFY) // [
  else if (upsampled)
    tree->stats.truepeak*=upsampled;
  else
    lib1770_pre_flush(track->filter.pre);
#else // ] [
  else if (!upsampled&&track->filter.pre)
    lib1770_pre_flush(track->filter.pre);
#endif // ]
#endif // ]

  return 0;
}

#if defined (FF_INPUT_CALLBACK_FLUSH) // [
static int input_stats_flush(void *data, int upsampled)
{
  bg_tree_t *tree=data;
  bg_track_t *track=&tree->track;

#if defined (FF_UPSAMPLE_MODIFY) // [
  if (upsampled)
    tree->stats.truepeak*=upsampled;
  else
    lib1770_pre_flush(track->filter.pre);
  else
#else // ] [
  if (!upsampled&&track->filter.pre)
    lib1770_pre_flush(track->filter.pre);
#endif // ]

  return 0;
}
#endif // ]

static ff_input_callback_t bg_input_callback={
  .path=input_path,
#if defined (_WIN32) // [
  .pathw=input_pathw,
#endif // ]
  .decode=input_decode,
  .upsample=input_upsample,
  .transcode=input_transcode,
  .csv=input_csv,
  .suppress_progress=input_suppress_progress,
  .interval.begin=input_interval_begin,
  .interval.duration=input_interval_duration,
  .stats.create=input_stats_create,
  .stats.destroy=input_stats_destroy,
  .stats.add=input_stats_add,
#if defined (FF_INPUT_CALLBACK_FLUSH) // [
  .stats.flush=input_stats_flush,
#endif // ]
};
