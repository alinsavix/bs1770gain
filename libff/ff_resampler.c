/*
 * ff_resampler.c
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
#include <ff.h>

///////////////////////////////////////////////////////////////////////////////
int ff_resampler_create(ff_resampler_t *res,
    const AVCodecParameters *ocodecpar,
    const AVCodecParameters *icodecpar)
{
  int64_t ochannel_layout=ocodecpar->channel_layout;

#if defined (FF_RESAMPLER_RATE) // [
  /////////////////////////////////////////////////////////////////////////////
  res->irate=icodecpar->sample_rate;
  res->orate=ocodecpar->sample_rate;
#endif // ]
#if defined (FF_RESAMPLER_NB_SAMPLES) // [
  res->nb_samples=0;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (!ochannel_layout||!icodecpar->channel_layout) {
    DVMESSAGE("invalid channel layout: output=%I64d input=%I64d",
        ochannel_layout,icodecpar->channel_layout);
    goto e_channel_layout;
  }

  /////////////////////////////////////////////////////////////////////////////
  res->ctx=swr_alloc_set_opts(
      NULL,                       // struct SwrContext *s,
      // [
      ochannel_layout,            // int64_t out_ch_layout,
      ocodecpar->format,          // enum AVSampleFormat out_sample_fmt,
      ocodecpar->sample_rate,     // int out_sample_rate,
      // ] [
      icodecpar->channel_layout,  // int64_t in_ch_layout,
      icodecpar->format,          // enum AVSampleFormat in_sample_fmt,
      icodecpar->sample_rate,     // int in_sample_rate,
      // ]
      0,                          // int log_obgset,
      NULL);                      // void *log_ctx

  if (!res->ctx) {
    DMESSAGE("allocating context");
    goto e_context;
  }

  /////////////////////////////////////////////////////////////////////////////
  res->frame=av_frame_alloc();

  if (!res->frame) {
    DMESSAGE("allocating frame");
    goto e_frame;
  }

  res->frame->channel_layout=ochannel_layout;
  res->frame->channels=av_get_channel_layout_nb_channels(ochannel_layout);
  res->frame->format=ocodecpar->format;
  res->frame->sample_rate=ocodecpar->sample_rate;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  av_frame_free(&res->frame);
e_frame:
  swr_close(res->ctx);
  swr_free(&res->ctx);
e_context:
e_channel_layout:
  return -1;
}

void resampler_destroy(ff_resampler_t *res)
{
  av_frame_free(&res->frame);
  swr_close(res->ctx);
  swr_free(&res->ctx);
}

//#define FF_RESAMPLER_GET_BUFFER
int resampler_apply(ff_resampler_t *res, AVFrame *frame)
{
  int err;

#if defined (FF_RESAMPLER_RATE) // [
  /////////////////////////////////////////////////////////////////////////////
  if (frame) {
    int nb_samples=(uint64_t)res->orate/res->irate*frame->nb_samples;

    if (!res->frame->nb_samples) {
#if defined (FF_RESAMPLER_NB_SAMPLES) // [
      res->frame->nb_samples=res->nb_samples=nb_samples;
#else // ] [
      res->frame->nb_samples=nb_samples;
#endif // ]

#if defined (FF_RESAMPLER_GET_BUFFER) // [
      err=av_frame_get_buffer(res->frame,0);

      if (err<0) {
        DMESSAGE("getting frame buffer");
        goto e_buffer;
      }
#endif // ]
    }
    else if (res->frame->nb_samples<nb_samples) {
#if defined (FF_RESAMPLER_NB_SAMPLES) // [
      if (res->nb_samples<nb_samples) {
#endif // ]
        uint64_t channel_layout=res->frame->channel_layout;
        int channels=res->frame->channels;
        int format=res->frame->format;
        int sample_rate=res->frame->sample_rate;

        av_frame_free(&res->frame);
        res->frame=av_frame_alloc();

        if (!res->frame) {
          DMESSAGE("allocating frame");
          goto e_frame;
        }

#if defined (FF_RESAMPLER_NB_SAMPLES) // [
        res->frame->nb_samples=res->nb_samples=nb_samples;
#else // ] [
        res->frame->nb_samples=nb_samples;
#endif // ]
        res->frame->channel_layout=channel_layout;
        res->frame->channels=channels;
        res->frame->format=format;
        res->frame->sample_rate=sample_rate;

#if defined (FF_RESAMPLER_GET_BUFFER) // [
        err=av_frame_get_buffer(res->frame,0);

        if (err<0) {
          DMESSAGE("getting frame buffer");
          goto e_buffer;
        }
#endif // ]
#if defined (FF_RESAMPLER_NB_SAMPLES) // [
      }
      else
        res->frame->nb_samples=nb_samples;
#endif // ]
    }
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  err=swr_convert_frame(res->ctx,res->frame,frame);

  if (err<0) {
    DVMESSAGE("converting frame: %s (%d)",av_err2str(err),err);
    goto econvert;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
econvert:
#if defined (FF_RESAMPLER_RATE) // [
#if defined (FF_RESAMPLER_GET_BUFFER) // [
e_buffer:
#endif // ]
e_frame:
#endif // ]
  return -1;
}
