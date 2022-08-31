/*
 * ff_analyzer.c
 *
 * Copyright (C) 2019 Peter Belkner <info@pbelkner.de>
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
#define FF_ANALYZER_CREATE_OCHANNELS
#define FF_ANALYZER_OCODECPAR_PTR
int ff_analyzer_create(ff_analyzer_t *a, ff_inout_t *in)
{
  ff_input_callback_t *cb=in->cb.in;
  void *data=in->cb.data;
  int upsample=cb&&cb->upsample?cb->upsample(data):-1;
  const ff_param_decode_t *(*decode)(const void *)=cb?cb->decode:NULL;
  int64_t channel_layout=decode?decode(data)->request.channel_layout:-1ll;
  int (*create)(void *, const AVCodecParameters *)=cb?cb->stats.create:NULL;
#if defined (FF_ANALYZER_OCODECPAR_PTR) // [
  AVCodecParameters *icodecpar,*ocodecpar;
#else // ] [
  AVCodecParameters *icodecpar,ocodecpar;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (in->ai<0) {
    _DMESSAGE("input not initialized");
    goto e_args;
  }

  /////////////////////////////////////////////////////////////////////////////
  icodecpar=in->fmt.ctx->streams[in->ai]->codecpar;

  if (!icodecpar->channel_layout) {
    _DMESSAGE("missing input channel layout");
    goto e_args;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (!channel_layout) {
    _DMESSAGE("missing output channel layout");
    goto e_args;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (!in->audio.ctx) {
    _DMESSAGE("audio decoder not opened");
    goto e_args;
  }

  /////////////////////////////////////////////////////////////////////////////
  a->state=FF_ANALYZER_DECODER_SEND_PACKET;
  a->in=in;

  /////////////////////////////////////////////////////////////////////////////
  if (0ll<channel_layout&&channel_layout!=(int)icodecpar->channel_layout) {
#if defined (FF_ANALYZER_OCODECPAR_PTR) // [
    /*
     * from "avcodec.h":
     *
     * sizeof(AVCodecParameters) is not a part of the public ABI,
     * this struct must be allocated with avcodec_parameters_alloc()
     * and freed with avcodec_parameters_free().
     */
    ocodecpar=avcodec_parameters_alloc();

    if (!ocodecpar) {
      _DMESSAGE("ocodecpar");
      goto e_normalizer_ocodecpar;
    }

    if (avcodec_parameters_copy(ocodecpar,icodecpar)<0) {
      _DMESSAGE("copying codecpar");
      avcodec_parameters_free(&ocodecpar);
      goto e_normalizer_ocodecpar_copy;
    }

    ocodecpar->channel_layout=channel_layout;
#if defined (FF_ANALYZER_CREATE_OCHANNELS) // [
    ocodecpar->channels=av_get_channel_layout_nb_channels(channel_layout);
#endif // ]

    ///////////////////////////////////////////////////////////////////////////
    if (ff_resampler_create(&a->normalizer,ocodecpar,icodecpar)<0) {
      _DMESSAGE("creating normalizer");
      avcodec_parameters_free(&ocodecpar);
      goto e_normalizer;
    }

    ///////////////////////////////////////////////////////////////////////////
    if (create&&create(data,ocodecpar)<0) {
      _DMESSAGE("creating statistics");
      avcodec_parameters_free(&ocodecpar);
      goto e_stats;
    }

    avcodec_parameters_free(&ocodecpar);
#else // ] [
    ocodecpar=*icodecpar;
    ocodecpar.channel_layout=channel_layout;
#if defined (FF_ANALYZER_CREATE_OCHANNELS) // [
    ocodecpar.channels=av_get_channel_layout_nb_channels(channel_layout);
#endif // ]

    ///////////////////////////////////////////////////////////////////////////
    if (ff_resampler_create(&a->normalizer,&ocodecpar,icodecpar)<0) {
      _DMESSAGE("creating normalizer");
      goto e_normalizer;
    }

    ///////////////////////////////////////////////////////////////////////////
    if (create&&create(data,&ocodecpar)<0) {
      _DMESSAGE("creating statistics");
      goto e_stats;
    }
#endif // ]
  }
  else {
    ///////////////////////////////////////////////////////////////////////////
    a->normalizer.ctx=NULL;

    ///////////////////////////////////////////////////////////////////////////
    if (create&&create(data,icodecpar)<0) {
      _DMESSAGE("creating statistics");
      goto e_stats;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  a->pkt=av_packet_alloc();

  if (!a->pkt) {
    _DMESSAGE("allocating packet");
    goto e_packet;
  }

  /////////////////////////////////////////////////////////////////////////////
  a->frame=av_frame_alloc();

  if (!a->frame) {
    _DMESSAGE("allocating frame");
    goto e_frame;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (1<upsample) {
#if defined (FF_ANALYZER_OCODECPAR_PTR) // [
    /*
     * from "avcodec.h":
     *
     * sizeof(AVCodecParameters) is not a part of the public ABI,
     * this struct must be allocated with avcodec_parameters_alloc()
     * and freed with avcodec_parameters_free().
     */
    ocodecpar=avcodec_parameters_alloc();

    if (!ocodecpar) {
      _DMESSAGE("ocodecpar");
      goto e_upsampler_ocodecpar;
    }

    if (avcodec_parameters_copy(ocodecpar,icodecpar)<0) {
      _DMESSAGE("copying codecpar");
      avcodec_parameters_free(&ocodecpar);
      goto e_upsampler_ocodecpar_copy;
    }

    ocodecpar->sample_rate*=upsample;
    ocodecpar->format=AV_SAMPLE_FMT_DBLP;

    if (ff_resampler_create(&a->upsampler,ocodecpar,icodecpar)<0) {
      _DMESSAGE("creating upsampler");
      avcodec_parameters_free(&ocodecpar);
      goto e_upsampler;
    }

    avcodec_parameters_free(&ocodecpar);
#else // ] [
    ocodecpar=*icodecpar;
    ocodecpar.sample_rate*=upsample;
    ocodecpar.format=AV_SAMPLE_FMT_DBLP;

    if (ff_resampler_create(&a->upsampler,&ocodecpar,icodecpar)<0) {
      _DMESSAGE("creating upsampler");
      goto e_upsampler;
    }
#endif // ]
  }
  else
    a->upsampler.ctx=NULL;

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  if (a->upsampler.ctx)
    resampler_destroy(&a->upsampler);
e_upsampler:
#if defined (FF_ANALYZER_OCODECPAR_PTR) // [
e_upsampler_ocodecpar_copy:
e_upsampler_ocodecpar:
#endif // ]
  av_frame_free(&a->frame);
e_frame:
  av_packet_free(&a->pkt);
e_packet:
  if (cb&&cb->stats.destroy)
    cb->stats.destroy(data);
e_stats:
  if (a->normalizer.ctx)
    resampler_destroy(&a->normalizer);
e_normalizer:
#if defined (FF_ANALYZER_OCODECPAR_PTR) // [
e_normalizer_ocodecpar_copy:
e_normalizer_ocodecpar:
#endif // ]
e_args:
  return -1;
}

void ff_analyzer_destroy(ff_analyzer_t *a, int destroy_stats)
{
  ff_input_callback_t *cb=a->in->cb.in;
  void *data=a->in->cb.data;

  if (a->upsampler.ctx)
    resampler_destroy(&a->upsampler);

  av_frame_free(&a->frame);
  av_packet_free(&a->pkt);

  if (destroy_stats&&cb&&cb->stats.destroy)
    cb->stats.destroy(data);

  if (a->normalizer.ctx)
   resampler_destroy(&a->normalizer);
}

///////////////////////////////////////////////////////////////////////////////
static int ff_analyzer_process_frame(ff_analyzer_t *a, AVFrame *frame)
{
  ff_input_callback_t *cb=a->in->cb.in;
  void *data=a->in->cb.data;
  int (*add)(void *, int, AVFrame *)=cb?cb->stats.add:NULL;
  int err;

  if (a->normalizer.ctx) {
    // normalize.
    err=resampler_apply(&a->normalizer,frame);

    if (err<0) {
      _DMESSAGEV("applying normalizer: %s (%d)",av_err2str(err),err);
      goto exit;
    }

    if (add) {
      // add to normalized non-upsample statistics.
      err=add(data,0,a->normalizer.frame);

      if (err<0) {
        _DMESSAGEV("adding normalized statistics: %s (%d)",
            av_err2str(err),err);
        goto exit;
      }

      if (!frame) {
        // we need to flush normalized non-upsample statistics.
        err=add(data,0,NULL);

        if (err<0) {
          _DMESSAGEV("flushing normalized statistics: %s (%d)",
              av_err2str(err),err);
          goto exit;
        }
      }
    }
  }
  else if (add) {
    // add non-normalized non-upsample statistics.
    err=add(data,0,frame);

    if (err<0) {
      _DMESSAGEV("adding statistics: %s (%d)",av_err2str(err),err);
      goto exit;
    }
  }

  if (a->upsampler.ctx) {
    // upsample.
    err=resampler_apply(&a->upsampler,frame);

    if (err<0) {
      _DMESSAGEV("applying upsampler: %s (%d)",av_err2str(err),err);
      goto exit;
    }

    if (add) {
      // add to upsample statistics.
      err=add(data,1,a->upsampler.frame);

      if (err<0) {
        _DMESSAGEV("adding upsampled statistics: %s (%d)",
            av_err2str(err),err);
        goto exit;
      }
    }

      if (!frame) {
        // we need to flush upsample statistics.
        err=add(data,1,NULL);

        if (err<0) {
          _DMESSAGEV("flushing upsampled statistics: %s (%d)",
              av_err2str(err),err);
          goto exit;
        }
      }
  }

  err=0;
exit:
  return err;
}

static int ff_analyzer_send_packet(ff_analyzer_t *a, AVPacket *pkt)
{
  int err=-1;

  for (;;) {
    switch (a->state) {
    case FF_ANALYZER_DECODER_SEND_PACKET:
      err=avcodec_send_packet(a->in->audio.ctx,pkt);

#if defined (FF_PACKET_UNREF) // [
      if (pkt)
        av_packet_unref(pkt);
#endif // ]

      switch (err) {
      case AVERROR_EOF:
        goto e_loop;
      case AVERROR_INVALIDDATA:
        // Invalid data found when processing input.
        // No respective documentation found. May occur at the end of MP3.
        // Intentional fall-through.
        continue;
      case 0:
        // 0 on success
        a->state=FF_ANALYZER_DECODER_RECEIVE_FRAME;
        continue;
      default:
        _DMESSAGEV("sending packet: %s (%d)",av_err2str(err),err);
        return err;
      }
    case FF_ANALYZER_DECODER_RECEIVE_FRAME:
      err=avcodec_receive_frame(a->in->audio.ctx,a->frame);

      switch (err) {
      case 0:
        // 0: success, a frame was returned we need to process.
        err=ff_analyzer_process_frame(a,a->frame);
#if defined (FF_FRAME_UNREF) // [
        av_frame_unref(a->frame);
#endif // ]

        if (err<0) {
          _DMESSAGEV("processing frame: %s (%d)",av_err2str(err),err);
          goto e_loop;
        }

        continue;
      case AVERROR_EOF:
        // we need to flush processing.
        err=ff_analyzer_process_frame(a,NULL);

        if (err<0) {
          _DMESSAGEV("processing frame: %s (%d)",av_err2str(err),err);
          goto e_loop;
        }

        a->state=FF_ANALYZER_DECODER_SEND_PACKET;
        return err;
      case AVERROR(EAGAIN):
        // AVERROR(EAGAIN): output is not available in this state - user must
        // try to send new input
        // read the next packet.
        a->state=FF_ANALYZER_DECODER_SEND_PACKET;
        return err;
      default:
        _DMESSAGEV("receiving frame: %s (%d)",av_err2str(err),err);
        return err;
      }
    default:
      _DMESSAGE("unexpected state");
      goto e_loop;
    }
  }

  return 0;
e_loop:
  return err;
}

int ff_analyzer_loop(ff_analyzer_t *a)
{
  AVFormatContext *ctx=a->in->fmt.ctx;
  AVPacket *pkt=a->pkt;
  int err;
read:
  err=av_read_frame(ctx,pkt);

  if (err<0)
    goto eof;
  else if ((int)ctx->nb_streams<=pkt->stream_index) {
#if defined (FF_PACKET_UNREF) // [
    av_packet_unref(a->pkt);
#endif // ]
    goto read;
  }

  if (ff_input_progress(a->in,pkt)<0)
    goto eof;

  if (a->in->ai!=a->pkt->stream_index) {
    // a packet from a stream we're not interested in has to be skipped.
#if defined (FF_PACKET_UNREF) // [
    av_packet_unref(a->pkt);
#endif // ]
    goto read;
  }

  // an audio packet has been read which has to be decoded.
  // the packet is unrefed by analyzer_send_packet().
  err=ff_analyzer_send_packet(a,a->pkt);

  if (FF_ANALYZER_DECODER_SEND_PACKET==a->state) {
    switch (err) {
    case AVERROR(EAGAIN):
      goto read;
    case AVERROR_EOF:
      goto eof;
    default:
#if 0 // [
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\02_all_stand_up_never_say_never.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\03_the_oriental.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\01_blues_and_rhythm.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\04_creepin_up_on_you.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\06_solid_gold.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\07_green.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\05_heavy_traffic.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\08_jam_side_down.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\09_diggin_burt_bacharach.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\12_i_don_t_remember_anymore.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\10_do_it_again.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\11_another_day.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\13_money_don_t_matter.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
Error: decoding frame: Error number 0 occurred (0:"F:\r128\audio\contemp\status_quo\2002_heavy_traffic\14_rhythm_of_life.mp3"). (ff_analyzer.c:461:ff_analyzer_loop)
      _DMESSAGEV("decoding frame: %s (%d)",av_err2str(err),err);
      goto e_loop;
#else // ] [
      _DWARNINGV("decoding frame: %s (%d:\"%s\")",av_err2str(err),err,
          a->in->cb.in->path(a->in->cb.data));
      goto eof;
#endif // ]
    }
  }
eof:
  /////////////////////////////////////////////////////////////////////////////
  // we need to flush the decoder.
  err=ff_analyzer_send_packet(a,NULL);

  if (err<0&&FF_ANALYZER_DECODER_SEND_PACKET<a->state) {
    _DMESSAGEV("decoding frame: %s (%d)",av_err2str(err),err);
    goto e_loop;
  }

  /////////////////////////////////////////////////////////////////////////////
  ff_printer_flush(a->in->printer);

  /////////////////////////////////////////////////////////////////////////////
  return 0;
e_loop:
  return err;
}
