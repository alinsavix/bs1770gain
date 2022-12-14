/*
 * ff_inout.c
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
#include <ff.h>

FF_DISABLE_DEPRECATION_WARNINGS // [
///////////////////////////////////////////////////////////////////////////////
// https://0xdeafc0de.wordpress.com/2014/07/21/seeking-with-ffmpeg/
//#define FF_SEEK_DEFAULT
//#define FF_SEEK_FLAGS AVFMT_SEEK_TO_PTS
//#define FF_SEEK_FLAGS AVSEEK_FLAG_FRAME 
#define FF_SEEK_FLAGS AVSEEK_FLAG_ANY 
//#define FF_SEEK_FLAGS 0 
static int ff_input_seek(ff_inout_t *in, int64_t seek)
{
  int err;

  if (0ll<seek) {
    if (in->fmt.ctx->duration<=seek) {
      _DMESSAGE("attempt to seek out of range");
      goto e_seek;
    }

#if defined (FF_SEEK_DEFAULT) // [
    err=av_seek_frame(in->fmt.ctx,-1,seek,FF_SEEK_FLAGS);
#else // ] [
    if (0<in->vi) {
      seek/=AV_TIME_BASE;
      seek*=in->fmt.ctx->streams[in->vi]->time_base.den;
      seek/=in->fmt.ctx->streams[in->vi]->time_base.num;
      err=av_seek_frame(in->fmt.ctx,in->vi,seek,FF_SEEK_FLAGS);
    }
    else {
      seek/=AV_TIME_BASE;
      seek*=in->fmt.ctx->streams[in->ai]->time_base.den;
      seek/=in->fmt.ctx->streams[in->ai]->time_base.num;
      err=av_seek_frame(in->fmt.ctx,in->ai,seek,FF_SEEK_FLAGS);
    }
#endif // ]

    if (err<0) {
      _DMESSAGEV("seeking: %s (%d)",av_err2str(err),err);
      goto e_seek;
    }
  }

  return 0;
e_seek:
  return -1;
}

#if defined (FF_SERGEY_INDEX_BUGFIX) // [
#if defined (FF_INPUT_LIST) // [
int ff_input_create(ff_inout_t *in, ff_input_callback_t *cb, void *data,
    int warn, ff_printer_t *p, int list, int ai, int vi)
#else // ] [
int ff_input_create(ff_inout_t *in, ff_input_callback_t *cb, void *data,
    int warn, ff_printer_t *p, int ai, int vi)
#endif // ]
#else // ] [
#if defined (FF_INPUT_LIST) // [
int ff_input_create(ff_inout_t *in, ff_input_callback_t *cb, void *data,
    int warn, ff_printer_t *p, int list)
#else // ] [
int ff_input_create(ff_inout_t *in, ff_input_callback_t *cb, void *data,
    int warn, ff_printer_t *p)
#endif // ]
#endif // ]
{
  const char *path=cb&&cb->path?cb->path(data):"";
#if defined (_WIN32) // [
  const wchar_t *pathw=cb&&cb->pathw?cb->pathw(data):L"";
#endif // ]
  int csv=cb&&cb->csv?cb->csv(data):0;
  const ff_param_decode_t *decode=cb&&cb->decode?cb->decode(data):NULL;
  int64_t seek=cb&&cb->interval.begin?cb->interval.begin(data):-1ll;
  int i;
  const AVStream *istream;
  int err;

  /////////////////////////////////////////////////////////////////////////////
  in->cb.in=cb;
  in->cb.data=data;
  in->printer=p;
#if defined (FF_INPUT_LIST) // [
  in->list=list;
  in->path=path;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  in->fmt.ctx=NULL;
  err=avformat_open_input(&in->fmt.ctx,path,NULL,NULL);

  if (err<0) {
    if (warn)
      _DWARNINGV("opening input \"%s\": %s (%d)",path,av_err2str(err),err);

    goto e_context;
  }

  if (!strcmp("image2",in->fmt.ctx->iformat->name))
    goto e_image;

  /////////////////////////////////////////////////////////////////////////////
  // should be set before calling avformat_find_stream_info().
  // cf. e.g. https://code.videolan.org/libav/libav/commit/9dc0bc3dadbc01b350b84c0079de391cb6015094
  in->fmt.ctx->flags|=AVFMT_FLAG_GENPTS;
  err=avformat_find_stream_info(in->fmt.ctx,NULL);

  if (err<0) {
    if (warn) {
      _DMESSAGEV("finding stream info \"%s\": %s (%d)",path,
          av_err2str(err),err);
    }

    goto e_find;
  }

  /////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
  if (csv&&ff_csv2avdict(path,pathw,'\t',&in->fmt.ctx->metadata,0)<0) {
    _DMESSAGE("reading csv file");
    goto e_csv;
  }
#else // ] [
  if (csv&&ff_csv2avdict(path,'\t',&in->fmt.ctx->metadata,0)<0) {
    _DMESSAGE("reading csv file");
    goto e_csv;
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
#if defined (FF_SERGEY_INDEX_BUGFIX) // [
  in->ai=ai<(int)in->fmt.ctx->nb_streams?ai:-1;
  in->vi=vi<(int)in->fmt.ctx->nb_streams?ai:-1;
#else // ] [
  in->ai=-1;
  in->vi=-1;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (!in->fmt.ctx->nb_streams)
    goto e_invalid_format;
  else if (1u==in->fmt.ctx->nb_streams
      &&in->fmt.ctx->streams[0]->codecpar->codec_type!=AVMEDIA_TYPE_AUDIO)
    goto e_invalid_format;

  /////////////////////////////////////////////////////////////////////////////
  for (i=0;i<(int)in->fmt.ctx->nb_streams;++i) {
    istream=in->fmt.ctx->streams[i];

    if (istream->codecpar->codec_id<=AV_CODEC_ID_NONE)
      continue;
    else if (AV_CODEC_ID_FIRST_SUBTITLE<=istream->codecpar->codec_id)
      continue;

    switch (istream->codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
#if defined (FF_CHANNEL_LAYOUT_DEPRECATED) // [
      if (istream->codecpar->ch_layout.nb_channels<=0) {
        _DWARNINGV("channel underflow: %d",
						istream->codecpar->ch_layout.nb_channels);
        continue;
      }
      else if (AV_NUM_DATA_POINTERS<istream->codecpar->ch_layout.nb_channels) {
        _DWARNINGV("channel overflow: %d",
						istream->codecpar->ch_layout.nb_channels);
        continue;
      }
#else // ] [
      if (istream->codecpar->channels<=0) {
        _DWARNINGV("channel underflow: %d",istream->codecpar->channels);
        continue;
      }
      else if (AV_NUM_DATA_POINTERS<istream->codecpar->channels) {
        _DWARNINGV("channel overflow: %d",istream->codecpar->channels);
        continue;
      }
#endif // ]
#if defined (FF_SERGEY_INDEX_BUGFIX) // [
			else if (i<in->ai)
				continue;
#endif // ]
#if defined (FF_CHANNEL_LAYOUT_DEPRECATED) // [
      else if (in->ai<0&&istream->codecpar->ch_layout.nb_channels) {
        // if no audio stream found yet, pick this one (regardless whether
        // stereo or not).
        in->ai=i;
      }
      else if (2==istream->codecpar->ch_layout.nb_channels) {
        // if an audio stream alreay was found (and is non-stereo),
        // replace it by this stereo one.
        in->ai=i;
      }
#else // ] [
      else if (in->ai<0&&istream->codecpar->channels) {
        // if no audio stream found yet, pick this one (regardless whether
        // stereo or not).
        in->ai=i;
      }
      else if (2==istream->codecpar->channels) {
        // if an audio stream alreay was found (and is non-stereo),
        // replace it by this stereo one.
        in->ai=i;
      }
#endif // ]

      break;
    case AVMEDIA_TYPE_VIDEO:
//DVWRITELN("\"%s\"",avcodec_find_decoder(istream->codecpar->codec_id)->name);
//DVWRITELN("%d",istream->codecpar->codec_id);
#if defined (FF_SERGEY_INDEX_BUGFIX) // [
			if (i<in->vi)
				continue;
			else
#endif // ]
      if (in->vi<0) {
        // if no video stream found yet, pick this one.
        in->vi=i;
      }

      break;
    default:
      break;
    }
  }

  if (in->ai<0) {
    _DWARNINGV("missing valid audio stream in \"%s\"",path);
    goto e_no_audio;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (!decode)
    in->audio.ctx=NULL;
  else if (ff_audio_create(&in->audio,in,decode,NULL)<0) {
    _DMESSAGE("creating audio decoder");
    goto e_audio;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (ff_input_seek(in,seek)<0) {
    _DMESSAGE("seeking");
    goto e_seek;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_seek:
  if (in->audio.ctx)
    ff_audio_destroy(&in->audio);
e_audio:
e_no_audio:
e_find:
e_invalid_format:
e_csv:
e_image:
  avformat_close_input(&in->fmt.ctx);
e_context:
  return -1;
}

void ff_input_destroy(ff_inout_t *in)
{
  if (in->audio.ctx)
    ff_audio_destroy(&in->audio);

  avformat_close_input(&in->fmt.ctx);
}

int ff_input_open_analyzer(ff_inout_t *in)
{
  ff_input_callback_t *cb=in->cb.in;
  void *data=in->cb.data;
  const char *path=cb&&cb->path?cb->path(data):"";
#if defined (_WIN32) // [
  const wchar_t *pathw=cb&&cb->pathw?cb->pathw(data):L"";
#endif // ]
  int csv=cb&&cb->csv?cb->csv(data):0;
  const ff_param_decode_t *decode=cb&&cb->decode?cb->decode(data):NULL;
  int64_t seek=cb&&cb->interval.begin?cb->interval.begin(data):-1ll;
  int err;

  /////////////////////////////////////////////////////////////////////////////
  in->cb.in=cb;
  in->cb.data=data;
  ff_printer_clear(in->printer);

  /////////////////////////////////////////////////////////////////////////////
  in->fmt.ctx=NULL;
  err=avformat_open_input(&in->fmt.ctx,path,NULL,NULL);

  if (err<0) {
    _DWARNINGV("opening input \"%s\": %s (%d)",path,av_err2str(err),err);
    goto e_context;
  }

  /////////////////////////////////////////////////////////////////////////////
  // should be set before calling avformat_find_stream_info().
  // cf. e.g. https://code.videolan.org/libav/libav/commit/9dc0bc3dadbc01b350b84c0079de391cb6015094
  in->fmt.ctx->flags|=AVFMT_FLAG_GENPTS;
  err=avformat_find_stream_info(in->fmt.ctx,NULL);

  if (err<0) {
    _DMESSAGEV("finding stream info: %s (%d)",av_err2str(err),err);
    goto e_find;
  }

  /////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
  if (csv&&ff_csv2avdict(path,pathw,'\t',&in->fmt.ctx->metadata,0)<0) {
    _DMESSAGE("reading csv file");
    goto e_csv;
  }
#else // ] [
  if (csv&&ff_csv2avdict(path,'\t',&in->fmt.ctx->metadata,0)<0) {
    _DMESSAGE("reading csv file");
    goto e_csv;
  }
#endif // ]

#if 0 // [
  err=av_log_get_level();

  if (AV_LOG_INFO<=err) {
#endif // ]
    ///////////////////////////////////////////////////////////////////////////
    fflush(stderr);
    fflush(stdout);
    av_dump_format(in->fmt.ctx,0,path,0);
    fflush(stderr);
#if 0 // [
  }

  av_log_set_level(err);
  err=-1;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (!decode)
    in->audio.ctx=NULL;
  else if (ff_audio_create(&in->audio,in,decode,NULL)<0) {
    _DMESSAGE("creating audio decoder");
    goto e_audio;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (ff_input_seek(in,seek)<0) {
    _DMESSAGE("seeking");
    goto e_seek;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_seek:
  if (in->audio.ctx)
    ff_audio_destroy(&in->audio);
e_audio:
e_find:
e_csv:
  avformat_close_input(&in->fmt.ctx);
e_context:
  return -1;
}

#if defined (FF_FLAC_HACK) // [
// as it seems it is impossible to pass through a partial flac stream.
// our hack consists in reading/decoding and re-encoding/writing.
static int ff_input_flac_hack(ff_inout_t *in, int transcode,
    int64_t begin, int64_t duration, ff_param_decode_t *d)
{
  AVCodecParameters *codecpar=in->fmt.ctx->streams[in->ai]->codecpar;
  int hack=AV_CODEC_ID_FLAC==codecpar->codec_id   // FLAC stream
      &&!transcode                                // not transcoding anyway
      &&(0ll<begin||0ll<duration);                // partial stream

  if (hack&&d)
    d->request.sample_fmt=codecpar->format;

  return hack;
}

int ff_input_open_muxer(ff_inout_t *in, int *hack)
#else // ] [
int ff_input_open_muxer(ff_inout_t *in)
#endif // ]
{
  // called from the muxer, i.e. we have to decide wether to transcode.
  // this might be forced by a respective parameter from the muxer
  // or by a changed format.
  ff_input_callback_t *cb=in->cb.in;
  void *data=in->cb.data;
  const char *path=cb&&cb->path?cb->path(data):"";
#if defined (_WIN32) // [
  const wchar_t *pathw=cb&&cb->pathw?cb->pathw(data):L"";
#endif // ]
  int csv=cb&&cb->csv?cb->csv(data):0;
  const ff_param_decode_t *decode=cb&&cb->decode?cb->decode(data):NULL;
  int transcode=cb&&cb->transcode?cb->transcode(data):0;
  int64_t begin=cb&&cb->interval.begin?cb->interval.begin(data):-1ll;
#if defined (FF_FLAC_HACK) // [
  int64_t duration=cb&&cb->interval.duration?cb->interval.duration(data):-1ll;
  ff_param_decode_t d;
#endif // ]
  int err;

  /////////////////////////////////////////////////////////////////////////////
  ff_printer_clear(in->printer);

  /////////////////////////////////////////////////////////////////////////////
  if (in->ai<0) {
    _DMESSAGE("audio index out of range");
    goto e_range;
  }

  /////////////////////////////////////////////////////////////////////////////
  in->fmt.ctx=NULL;
  err=avformat_open_input(&in->fmt.ctx,path,NULL,NULL);

  if (err<0) {
    _DMESSAGEV("opening input: %s (%d) \"%s\"",av_err2str(err),err,path);
    goto e_context;
  }

  /////////////////////////////////////////////////////////////////////////////
  // should be set before calling avformat_find_stream_info().
  // cf. e.g. https://code.videolan.org/libav/libav/commit/9dc0bc3dadbc01b350b84c0079de391cb6015094
  in->fmt.ctx->flags|=AVFMT_FLAG_GENPTS;
  err=avformat_find_stream_info(in->fmt.ctx,NULL);

  if (err<0) {
    _DMESSAGEV("finding stream info: %s (%d)",av_err2str(err),err);
    goto e_find;
  }

  /////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
  if (csv&&ff_csv2avdict(path,pathw,'\t',&in->fmt.ctx->metadata,0)<0) {
    _DMESSAGE("reading csv file");
    goto e_csv;
  }
#else // ] [
  if (csv&&ff_csv2avdict(path,'\t',&in->fmt.ctx->metadata,0)<0) {
    _DMESSAGE("reading csv file");
    goto e_csv;
  }
#endif // ]

#if 0 // [
  err=av_log_get_level();

  if (AV_LOG_INFO<=err) {
#endif // ]
    ///////////////////////////////////////////////////////////////////////////
    fflush(stderr);
    fflush(stdout);
    av_dump_format(in->fmt.ctx,0,path,0);
    fflush(stderr);
#if 0 // [
  }

  av_log_set_level(err);
  err=-1;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
#if defined (FF_FLAC_HACK) // [
  if (decode) {
    d=*decode;

    if (hack)
      *hack=ff_input_flac_hack(in,transcode,begin,duration,&d);
  }
  else if (hack)
    *hack=0;

  if (!decode||!(transcode||(hack&&*hack)))
    in->audio.ctx=NULL;
  else if (ff_audio_create(&in->audio,in,&d,NULL)<0) {
      _DMESSAGE("creating audio decoder");
      goto e_audio;
  }
#else // ] [
  if (!decode||!transcode)
    in->audio.ctx=NULL;
  else if (ff_audio_create(&in->audio,in,decode,NULL)<0) {
    _DMESSAGE("creating audio decoder");
    goto e_audio;
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (ff_input_seek(in,begin)<0) {
    _DMESSAGE("seeking");
    goto e_seek;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
e_seek:
  if (in->audio.ctx)
    ff_audio_destroy(&in->audio);
e_audio:
e_csv:
e_find:
  avformat_close_input(&in->fmt.ctx);
e_context:
e_range:
  return -1;
}

void ff_input_close(ff_inout_t *in)
{
  if (in->audio.ctx) {
    ff_audio_destroy(&in->audio);
    in->audio.ctx=NULL;
  }

  if (in->fmt.ctx) {
    avformat_close_input(&in->fmt.ctx);
    in->fmt.ctx=NULL;
  }
}

void ff_inout_interval(ff_inout_t *inout, int64_t *begin, int64_t *duration,
    AVStream *stream)
{
  int64_t (*fn)(const void *data);

  if (begin) {
    fn=inout->cb.in?inout->cb.in->interval.begin:NULL;
    *begin=fn?fn(inout->cb.data):-1ll;
//DVWRITELN("%p: begin=%I64d",fn,*begin);
  }

  if (duration) {
    fn=inout->cb.in?inout->cb.in->interval.duration:NULL;
    *duration=fn?fn(inout->cb.data):-1ll;
//DVWRITELN("%p: duration=%I64d",fn,*duration);

    if (*duration<0ll) {
      *duration=inout->fmt.ctx->duration;
//DVWRITELN("%p: duration=%I64d",fn,*duration);

      if (begin&&0ll<*begin)
        *duration-=*begin;
//DVWRITELN("%p: duration=%I64d",fn,*duration);
    }
  }

//DVWRITELN("begin=%I64d",*begin);
  if (stream) {
    if (begin&&0ll<*begin)
      *begin=av_rescale_q(*begin,AV_TIME_BASE_Q,stream->time_base);

    if (duration&&0ll<*duration)
      *duration=av_rescale_q(*duration,AV_TIME_BASE_Q,stream->time_base);
  }
//DVWRITELN("begin=%I64d",*begin);
}


int ff_input_progress(ff_inout_t *in, AVPacket *pkt)
{
  enum { BUF_SIZE=64 };
  int supress_progress=in->cb.in&&in->cb.in->suppress_progress
      ?in->cb.in->suppress_progress(in->cb.data):0;
  AVStream *stream=in->fmt.ctx->streams[pkt->stream_index];
  int64_t duration,begin;
  double percent;

  ff_inout_interval(in,&begin,&duration,stream);
#if defined (FF_PROGRESS_STDERR) // [
  ff_printer_reset(in->printer,stderr);
#else // ] [
  ff_printer_reset(in->printer);
#endif // ]

  if (duration<0) {
    if (!supress_progress) {
#if defined (_WIN32) // [
      ff_printer_printf(in->printer,L"%I64d sec",
          pkt->pts*stream->time_base.num/stream->time_base.den);
#else // ] [
      ff_printer_printf(in->printer,"%" PRId64 " sec",
          pkt->pts*stream->time_base.num/stream->time_base.den);
#endif // ]
    }

    return 0;
  }

  if (0<=begin) {
    pkt->pts-=begin;
    pkt->dts-=begin;
  }

  percent=0ll<pkt->pts&&0ll<duration?100.0*pkt->pts/duration:0.0;

  if (!supress_progress)
      _FF_PRINTER_PRINTF(in->printer,"%.0f%%",percent);

  return duration<=pkt->pts?-1:0;
}

///////////////////////////////////////////////////////////////////////////////
#define FF_QUERY_CODEC
static int ff_output_add_vstream(ff_inout_t *out)
{
  ff_output_callback_t *ocb=out->cb.out;
  void *odata=out->cb.data;
  const ff_analyzer_t *a=ocb&&ocb->analyzer?ocb->analyzer(odata):NULL;
  ff_inout_t *in=a->in;
  AVStream *istream=in->fmt.ctx->streams[in->vi];
  AVStream *ostream;
  int err=-1;

#if defined (FF_QUERY_CODEC) // [
  // err is interpreted as a boolean.
  err=avformat_query_codec(out->fmt.ctx->oformat,
      istream->codecpar->codec_id,1);

  if (err) {
    err=-1;
#endif // ]
    istream=in->fmt.ctx->streams[in->vi];
    out->vi=out->fmt.ctx->nb_streams;

    // if succeeding the stream will be added to out->fmt.ctx and hence
    // will be destroyed by calling avformat_free_context().
    ostream=avformat_new_stream(out->fmt.ctx,NULL);

    if (!ostream) {
      _DMESSAGEV("creating video output stream: %s (%d)",av_err2str(err),err);
      goto e_video;
    }

    // we're going to remux.
    err=avcodec_parameters_copy(ostream->codecpar,istream->codecpar);

    if (err<0) {
      _DMESSAGE("copying codec parametrs");
      goto e_video;
    }

    ostream->id=out->vi;
    ostream->time_base=istream->time_base;
    ostream->codecpar->codec_tag=0;
    // needed to copy embedded album art:
    ostream->disposition=istream->disposition;
#if defined (FF_QUERY_CODEC) // [
  }
  else {
    // the video is lost.
    _DWARNING("unable to remux video stream");
    in->vi=-1;
  }
#endif // ]

  return 0;
e_video:
  return -1;
}

#define FF_CODECPAR_ALLOC
#if defined (FF_FLAC_HACK) // [
static int ff_output_add_astream(ff_inout_t *out,
    enum AVSampleFormat sample_fmt)
#else // ] [
static int ff_output_add_astream(ff_inout_t *out)
#endif // ]
{
  ff_output_callback_t *ocb=out->cb.out;
  void *odata=out->cb.data;
  const ff_analyzer_t *a=ocb&&ocb->analyzer?ocb->analyzer(odata):NULL;
  ff_inout_t *in=a->in;
  ff_input_callback_t *icb=in->cb.in;
  void *idata=in->cb.data;
  const ff_param_decode_t *decode=icb&&icb->decode?icb->decode(idata):NULL;
  int64_t channel_layout=decode?decode->request.channel_layout:-1ll;
  AVStream *istream=in->fmt.ctx->streams[in->ai];
  AVStream *ostream;
#if defined (FF_CODECPAR_ALLOC) // [
  // from "avcodec.h":
  // sizeof(AVCodecParameters) is not a part of the public ABI, this
  // struct must be allocated with avcodec_parameters_alloc() and
  // freed with avcodec_parameters_free().
  AVCodecParameters *codecpar;
#else // ] [
  AVCodecParameters codecpar;
#endif // ]
  int err=-1;

  out->ai=out->fmt.ctx->nb_streams;

  // if succeeding the stream will be added to out->fmt.ctx and hence
  // will be destroyed by calling avformat_free_context().
  ostream=avformat_new_stream(out->fmt.ctx,NULL);

  if (!ostream) {
    _DMESSAGEV("creating audio output stream: %s (%d)",av_err2str(err),err);
    goto e_audio;
  }

  ostream->id=out->ai;

  if (!in->audio.ctx) {
    out->audio.ctx=NULL;
    err=avcodec_parameters_copy(ostream->codecpar,istream->codecpar);

    if (err<0) {
      _DMESSAGE("copying codec parametrs");
      goto e_audio;
    }

    ostream->time_base=istream->time_base;
    ostream->codecpar->codec_tag=0;

    // "avformat.h" states w.r.t. field "duration" of "struct AVStream":
    // Encoding: May be set by the caller before avformat_write_header() to
    // provide a hint to the muxer about the estimated duration.
    //
    // Unfortunately this doesn't seem to work with FLAC streams when
    // picking some interval.
    // Cf. "https://trac.ffmpeg.org/ticket/7864".
    ff_inout_interval(in,NULL,&ostream->duration,ostream);
  }
  else {
#if defined (FF_FLAC_HACK) // [
    if (AV_SAMPLE_FMT_NONE==sample_fmt) {
      sample_fmt=ocb&&ocb->sample_fmt
          ?ocb->sample_fmt(odata):AV_SAMPLE_FMT_NONE;
    }
#endif // ]

#if defined (FF_CODECPAR_ALLOC) // [
    codecpar=avcodec_parameters_alloc();

    if (!codecpar) {
      _DMESSAGE("allocating codecpar");
      goto e_audio;
    }

#if 0 // [
    codecpar=*istream->codecpar;
#else // ] [
    if (avcodec_parameters_copy(codecpar,istream->codecpar)<0) {
      _DMESSAGE("copying codecpar");
      goto e_audio;
    }
#endif // ]

    codecpar->codec_id=ocb&&ocb->codec_id
        ?ocb->codec_id(odata,out->fmt.ctx->oformat):AV_CODEC_ID_FLAC;
    codecpar->format=sample_fmt;

#if defined (FF_CHANNEL_LAYOUT_DEPRECATED) // [
	#error not implemented yet.
#else // ] [
    if (0ll<=channel_layout)
      codecpar->channel_layout=channel_layout;
#endif // ]

    err=ff_audio_create(&out->audio,out,NULL,codecpar);
    avcodec_parameters_free(&codecpar);
#else // ] [
    codecpar=*istream->codecpar;
    codecpar.codec_id=ocb&&ocb->codec_id
        ?ocb->codec_id(odata,out->fmt.ctx->oformat):AV_CODEC_ID_FLAC;
    codecpar.format=sample_fmt;

    if (0ll<=channel_layout)
      codecpar.channel_layout=channel_layout;

    err=ff_audio_create(&out->audio,out,NULL,&codecpar);
#endif // ]
    fflush(stderr);

    if (err<0) {
      _DMESSAGEV("creating audio encoder: %s (%d)",av_err2str(err),err);
      goto e_audio;
    }
  }

  return 0;
e_audio:
  return -1;
}

#if defined (FF_INPUT_LIST) // [
#if defined (FF_FLAC_HACK) // [
int ff_output_create(ff_inout_t *out, ff_output_callback_t *ocb, void *odata,
    enum AVSampleFormat sample_fmt, int list)
#else // ] [
int ff_output_create(ff_inout_t *out, ff_output_callback_t *ocb, void *odata,
    int list)
#endif // ]
#else // ] [
#if defined (FF_FLAC_HACK) // [
int ff_output_create(ff_inout_t *out, ff_output_callback_t *ocb, void *odata,
    enum AVSampleFormat sample_fmt)
#else // ] [
int ff_output_create(ff_inout_t *out, ff_output_callback_t *ocb, void *odata)
#endif // ]
#endif // ]
{
  const char *path=ocb&&ocb->path?ocb->path(odata):"";
  const ff_analyzer_t *a=ocb&&ocb->analyzer?ocb->analyzer(odata):NULL;
#if ! defined (FF_FLAC_HACK) // [
  enum AVSampleFormat sample_fmt
      =ocb&&ocb->sample_fmt?ocb->sample_fmt(odata):AV_SAMPLE_FMT_NONE;
#endif // ]
  ff_inout_t *in=a->in;
#if defined (FF_STREAM_METADATA) // [
  void (*metadata)(void *,AVDictionary **,const AVDictionary *,
      ff_metadata_type_t)=ocb?ocb->metadata:NULL;
#else // ] [
  void (*metadata)(void *,AVDictionary **,const AVDictionary *)=
      ocb?ocb->metadata:NULL;
#endif // ]
  int err;

  /////////////////////////////////////////////////////////////////////////////
  out->cb.out=ocb;
  out->cb.data=odata;
  out->printer=NULL;
  out->ai=-1;
  out->vi=-1;
#if defined (FF_INPUT_LIST) // [
  out->list=list;
  out->path=path;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  out->fmt.ctx=NULL;
//_DWRITELNV("\"%s\"",path);
  err=avformat_alloc_output_context2(&out->fmt.ctx,NULL,NULL,path);

  if (err<0) {
    _DMESSAGEV("allocating output context \"%s\": %s (%d)",path,
        av_err2str(err),err);
    goto e_format;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (0<=in->vi&&in->vi<in->ai) {
    err=ff_output_add_vstream(out);
    
    if (err<0) {
      _DMESSAGE("creating video output stream");
      goto e_video1;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  if (0<=in->ai) {
#if defined (FF_FLAC_HACK) // [
    err=ff_output_add_astream(out,sample_fmt);
#else // ] [
    err=ff_output_add_astream(out);
#endif // ]
    
    if (err<0) {
      _DMESSAGE("creating audio output stream");
      goto e_audio;
    }
  }

  if (0<=in->ai&&in->ai<in->vi) {
    err=ff_output_add_vstream(out);

    if (err<0) {
      _DMESSAGE("creating video output stream");
      goto e_video2;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  if (!(out->fmt.ctx->oformat->flags&AVFMT_NOFILE)) {
    err=avio_open(&out->fmt.ctx->pb,path,AVIO_FLAG_WRITE);

    if (err<0) {
      _DMESSAGEV("open output file \"%s\": %s (%d)",path,av_err2str(err),err);
      goto e_open;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  if (metadata) {
#if defined (FF_STREAM_METADATA) // [
    if (0<=out->vi&&0<=in->vi) {
      metadata(odata,&out->fmt.ctx->streams[out->vi]->metadata,
          in->fmt.ctx->streams[in->vi]->metadata,FF_METADATA_TYPE_VIDEO);
    }

    if (0<=out->ai&&0<=in->ai) {
      metadata(odata,&out->fmt.ctx->streams[out->ai]->metadata,
          in->fmt.ctx->streams[in->ai]->metadata,FF_METADATA_TYPE_AUDIO);
    }

    metadata(odata,&out->fmt.ctx->metadata,in->fmt.ctx->metadata,
        FF_METADATA_TYPE_FORMAT);
#else // ] [
    metadata(odata,&out->fmt.ctx->metadata,in->fmt.ctx->metadata);
#endif // ]
  }

  /////////////////////////////////////////////////////////////////////////////
  err=avformat_write_header(out->fmt.ctx,NULL);

  if (err<0) {
    _DMESSAGEV("writing header \"%s\": %s (%d)",path,av_err2str(err),err);
    goto e_header;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
  av_write_trailer(out->fmt.ctx);
e_header:
  if (!(out->fmt.ctx->oformat->flags&AVFMT_NOFILE))
    avio_closep(&out->fmt.ctx->pb);
e_open:
  if (out->audio.ctx)
    ff_audio_destroy(&out->audio);
e_video2:
e_audio:
e_video1:
  avformat_free_context(out->fmt.ctx);
e_format:
  return -1;
}

#if defined (FF_INPUT_LIST) // [
void ff_inout_list(ff_inout_t *inout, int out)
{
  int err=av_log_get_level();

  /////////////////////////////////////////////////////////////////////////////
#if defined (FF_INPUT_LIST) // [
  if (inout->list||AV_LOG_INFO<=err) {
#else // ] [
  if (AV_LOG_INFO<=err) {
#endif // ]
    fflush(stdout);
    fflush(stderr);

    if (err<AV_LOG_INFO)
      av_log_set_level(AV_LOG_INFO);

    av_dump_format(inout->fmt.ctx,0,inout->path,out);
    fflush(stderr);

    if (err<AV_LOG_INFO)
      av_log_set_level(err);
#if ! defined (FF_INPUT_LIST) // [
  }
#else // ] [
  }
#endif // ]
}
#endif // ]

void ff_output_destroy(ff_inout_t *out)
{
  av_write_trailer(out->fmt.ctx);

  if (!(out->fmt.ctx->oformat->flags&AVFMT_NOFILE))
    avio_closep(&out->fmt.ctx->pb);

  if (out->audio.ctx)
    ff_audio_destroy(&out->audio);

  avformat_free_context(out->fmt.ctx);
}

FF_ENABLE_DEPRECATION_WARNINGS // ]
