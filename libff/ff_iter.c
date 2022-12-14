/*
 * ff_iter.h
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
#include <lib1770.h>
#include <ff.h>

FF_DISABLE_DEPRECATION_WARNINGS // [
///////////////////////////////////////////////////////////////////////////////
static ff_iter_vmt_t ff_iter_s16i_vmt;
static ff_iter_vmt_t ff_iter_s32i_vmt;
static ff_iter_vmt_t ff_iter_flti_vmt;
static ff_iter_vmt_t ff_iter_dbli_vmt;
static ff_iter_vmt_t ff_iter_s16p_vmt;
static ff_iter_vmt_t ff_iter_s32p_vmt;
static ff_iter_vmt_t ff_iter_fltp_vmt;
static ff_iter_vmt_t ff_iter_dblp_vmt;
static ff_iter_vmt_t ff_iter_err_vmt;

///////////////////////////////////////////////////////////////////////////////
int ff_iter_first(ff_iter_t *i, const AVFrame *frame)
{
  i->frame=frame;

  switch (frame->format) {
  /////////////////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_NONE:
    _DMESSAGE("format AV_SAMPLE_FMT_NONE not supported");
    goto exit;
  // interleaved //////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8:
    _DMESSAGE("format AV_SAMPLE_FMT_U8 not supported");
    goto exit;
  case AV_SAMPLE_FMT_S16:
    i->vmt=&ff_iter_s16i_vmt;
    break;
  case AV_SAMPLE_FMT_S32:
    i->vmt=&ff_iter_s32i_vmt;
    break;
  case AV_SAMPLE_FMT_FLT:
    i->vmt=&ff_iter_flti_vmt;
    break;
  case AV_SAMPLE_FMT_DBL:
    i->vmt=&ff_iter_dbli_vmt;
    break;
  // planar ///////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8P:
    _DMESSAGE("format AV_SAMPLE_FMT_U8P not supported");
    goto exit;
  case AV_SAMPLE_FMT_S16P:
    i->vmt=&ff_iter_s16p_vmt;
    break;
  case AV_SAMPLE_FMT_S32P:
    i->vmt=&ff_iter_s32p_vmt;
    break;
  case AV_SAMPLE_FMT_FLTP:
    i->vmt=&ff_iter_fltp_vmt;
    break;
  case AV_SAMPLE_FMT_DBLP:
    i->vmt=&ff_iter_dblp_vmt;
    break;
  /////////////////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_NB:
    // Number of sample formats. DO NOT USE if linking dynamically.
    _DMESSAGE("format AV_SAMPLE_FMT_NB not supported");
    goto exit;
  default:
    _DMESSAGEV("format %d not supported",frame->format);
    goto exit;
  }

  /////////////////////////////////////////////////////////////////////////////
  i->vmt->first(i);

  /////////////////////////////////////////////////////////////////////////////
  return 0;
exit:
  i->vmt=&ff_iter_err_vmt;
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
#define FF_INT16_SCALE (1.0/PBU_MAXOF(int16_t))
#define FF_INT32_SCALE (1.0/PBU_MAXOF(int32_t))
#define FF_INT16_SCALE_NEG (1.0/(1.0+PBU_MAXOF(int16_t)))
#define FF_INT32_SCALE_NEG (1.0/(1.0+PBU_MAXOF(int32_t)))

static void ff_iter_norm(ff_iter_t *i FFUNUSED, double *x, double *max,
    int channels, int ch, double sample)
{
  if (x) {
    if (channels<FF_LFE_THRESHOLD||ch<FF_LFE_CHANNEL) {
      x[ch]=sample;
    }
    else if (LIB1770_MAX_CHANNELS<ch) {
      _DWARNINGV("channel overflow: %d",ch);
      return;
    }
    else if (FF_LFE_CHANNEL<ch) {
_DWRITELNV("+++ ch:%d: sample:%lf",ch-1,sample);
      x[ch-1]=sample;
    }
  }

  if (max) {
    if (sample<0.0) {
      if (sample<-*max)
        *max=-sample;
//DVWRITELN("%lf %lf",-sample,*max);
    }
    else if (*max<sample) {
      *max=sample;
//DVWRITELN("%lf %lf",sample,*max);
    }
//DVWRITELN("--- %lf",*max);
  }
}

///////////////////////////////////////////////////////////////////////////////
static void ff_iter_s16i_first(ff_iter_t *i)
{
#if defined (__GNUC__) // [
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif // ]
  i->i.s16.rp=(int16_t *)i->frame->data[0];
#if defined (__GNUC__) // [
#pragma GCC diagnostic pop
#endif // ]
  i->i.s16.mp=i->i.s16.rp+i->frame->channels*i->frame->nb_samples;
}

static int ff_iter_s16i_valid(ff_iter_t *i)
{
  return i->i.s16.rp<i->i.s16.mp;
}

static void ff_iter_s16i_next(ff_iter_t *i)
{
  i->i.s16.rp+=i->frame->channels;
}

static void ff_iter_s16i_norm(ff_iter_t *i, double *x, double *max)
{
  int ch;
  int16_t sample;

  for (ch=0;ch<i->frame->channels;++ch) {
    sample=i->i.s16.rp[ch];

    if (sample<0)
      ff_iter_norm(i,x,max,i->frame->channels,ch,FF_INT16_SCALE_NEG*sample);
    else
      ff_iter_norm(i,x,max,i->frame->channels,ch,FF_INT16_SCALE*sample);
  }
}

static ff_iter_vmt_t ff_iter_s16i_vmt={
  .id="s16i",
  .first=ff_iter_s16i_first,
  .valid=ff_iter_s16i_valid,
  .next=ff_iter_s16i_next,
  .norm=ff_iter_s16i_norm,
};

///////////////////////////////////////////////////////////////////////////////
static void ff_iter_s32i_first(ff_iter_t *i)
{
#if defined (__GNUC__) // [
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif // ]
  i->i.s32.rp=(int32_t *)i->frame->data[0];
#if defined (__GNUC__) // [
#pragma GCC diagnostic pop
#endif // ]
  i->i.s32.mp=i->i.s32.rp+i->frame->channels*i->frame->nb_samples;
}

static int ff_iter_s32i_valid(ff_iter_t *i)
{
  return i->i.s32.rp<i->i.s32.mp;
}

static void ff_iter_s32i_next(ff_iter_t *i)
{
  i->i.s32.rp+=i->frame->channels;
}

static void ff_iter_s32i_norm(ff_iter_t *i, double *x, double *max)
{
  int ch;
  int32_t sample;

  for (ch=0;ch<i->frame->channels;++ch) {
    sample=i->i.s32.rp[ch];

    if (sample<0)
      ff_iter_norm(i,x,max,i->frame->channels,ch,FF_INT32_SCALE_NEG*sample);
    else
      ff_iter_norm(i,x,max,i->frame->channels,ch,FF_INT32_SCALE*sample);
  }
}

static ff_iter_vmt_t ff_iter_s32i_vmt={
  .id="s32i",
  .first=ff_iter_s32i_first,
  .valid=ff_iter_s32i_valid,
  .next=ff_iter_s32i_next,
  .norm=ff_iter_s32i_norm,
};

///////////////////////////////////////////////////////////////////////////////
static void ff_iter_flti_first(ff_iter_t *i)
{
#if defined (__GNUC__) // [
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif // ]
  i->i.flt.rp=(float *)i->frame->data[0];
#if defined (__GNUC__) // [
#pragma GCC diagnostic pop
#endif // ]
  i->i.flt.mp=i->i.flt.rp+i->frame->channels*i->frame->nb_samples;
}

static int ff_iter_flti_valid(ff_iter_t *i)
{
  return i->i.flt.rp<i->i.flt.mp;
}

static void ff_iter_flti_next(ff_iter_t *i)
{
  i->i.flt.rp+=i->frame->channels;
}

static void ff_iter_flti_norm(ff_iter_t *i, double *x, double *max)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch)
    ff_iter_norm(i,x,max,i->frame->channels,ch,i->i.flt.rp[ch]);
}

static ff_iter_vmt_t ff_iter_flti_vmt={
  .id="flti",
  .first=ff_iter_flti_first,
  .valid=ff_iter_flti_valid,
  .next=ff_iter_flti_next,
  .norm=ff_iter_flti_norm,
};

///////////////////////////////////////////////////////////////////////////////
static void ff_iter_dbli_first(ff_iter_t *i)
{
#if defined (__GNUC__) // [
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif // ]
  i->i.dbl.rp=(double *)i->frame->data[0];
#if defined (__GNUC__) // [
#pragma GCC diagnostic pop
#endif // ]
  i->i.dbl.mp=i->i.dbl.rp+i->frame->channels*i->frame->nb_samples;
}

static int ff_iter_dbli_valid(ff_iter_t *i)
{
  return i->i.dbl.rp<i->i.dbl.mp;
}

static void ff_iter_dbli_next(ff_iter_t *i)
{
  i->i.dbl.rp+=i->frame->channels;
}

static void ff_iter_dbli_norm(ff_iter_t *i, double *x, double *max)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch)
    ff_iter_norm(i,x,max,i->frame->channels,ch,i->i.dbl.rp[ch]);
}

static ff_iter_vmt_t ff_iter_dbli_vmt={
  .id="dbli",
  .first=ff_iter_dbli_first,
  .valid=ff_iter_dbli_valid,
  .next=ff_iter_dbli_next,
  .norm=ff_iter_dbli_norm,
};

///////////////////////////////////////////////////////////////////////////////
static void ff_iter_s16p_first(ff_iter_t *i)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch) {
#if defined (__GNUC__) // [
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif // ]
    i->p.s16.rp[ch]=(int16_t *)i->frame->data[ch];
#if defined (__GNUC__) // [
#pragma GCC diagnostic pop
#endif // ]
  }

  i->p.s16.mp=i->p.s16.rp[0]+i->frame->nb_samples;
}

static int ff_iter_s16p_valid(ff_iter_t *i)
{
  return i->p.s16.rp[0]<i->p.s16.mp;
}

static void ff_iter_s16p_next(ff_iter_t *i)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch)
    ++i->p.s16.rp[ch];
}

static void ff_iter_s16p_norm(ff_iter_t *i, double *x, double *max)
{
  int ch;
  int16_t sample;

  for (ch=0;ch<i->frame->channels;++ch) {
    sample=*i->p.s16.rp[ch];

    if (sample<0)
      ff_iter_norm(i,x,max,i->frame->channels,ch,FF_INT16_SCALE_NEG*sample);
    else
      ff_iter_norm(i,x,max,i->frame->channels,ch,FF_INT16_SCALE*sample);
  }
}

static ff_iter_vmt_t ff_iter_s16p_vmt={
  .id="s16p",
  .first=ff_iter_s16p_first,
  .valid=ff_iter_s16p_valid,
  .next=ff_iter_s16p_next,
  .norm=ff_iter_s16p_norm,
};

///////////////////////////////////////////////////////////////////////////////
static void ff_iter_s32p_first(ff_iter_t *i)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch) {
#if defined (__GNUC__) // [
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif // ]
    i->p.s32.rp[ch]=(int32_t *)i->frame->data[ch];
#if defined (__GNUC__) // [
#pragma GCC diagnostic pop
#endif // ]
  }

  i->p.s32.mp=i->p.s32.rp[0]+i->frame->nb_samples;
}

static int ff_iter_s32p_valid(ff_iter_t *i)
{
  return i->p.s32.rp[0]<i->p.s32.mp;
}

static void ff_iter_s32p_next(ff_iter_t *i)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch)
    ++i->p.s32.rp[ch];
}

static void ff_iter_s32p_norm(ff_iter_t *i, double *x, double *max)
{
  int ch;
  int32_t sample;

  for (ch=0;ch<i->frame->channels;++ch) {
    sample=*i->p.s32.rp[ch];

    if (sample<0)
      ff_iter_norm(i,x,max,i->frame->channels,ch,FF_INT32_SCALE_NEG*sample);
    else
      ff_iter_norm(i,x,max,i->frame->channels,ch,FF_INT32_SCALE*sample);
  }
}

static ff_iter_vmt_t ff_iter_s32p_vmt={
  .id="s32p",
  .first=ff_iter_s32p_first,
  .valid=ff_iter_s32p_valid,
  .next=ff_iter_s32p_next,
  .norm=ff_iter_s32p_norm,
};

///////////////////////////////////////////////////////////////////////////////
static void ff_iter_fltp_first(ff_iter_t *i)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch) {
#if defined (__GNUC__) // [
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif // ]
    i->p.flt.rp[ch]=(float *)i->frame->data[ch];
#if defined (__GNUC__) // [
#pragma GCC diagnostic pop
#endif // ]
  }

  i->p.flt.mp=i->p.flt.rp[0]+i->frame->nb_samples;
}

static int ff_iter_fltp_valid(ff_iter_t *i)
{
  return i->p.flt.rp[0]<i->p.flt.mp;
}

static void ff_iter_fltp_next(ff_iter_t *i)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch)
    ++i->p.flt.rp[ch];
}

static void ff_iter_fltp_norm(ff_iter_t *i, double *x, double *max)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch)
    ff_iter_norm(i,x,max,i->frame->channels,ch,*i->p.flt.rp[ch]);
}

static ff_iter_vmt_t ff_iter_fltp_vmt={
  .id="fltp",
  .first=ff_iter_fltp_first,
  .valid=ff_iter_fltp_valid,
  .next=ff_iter_fltp_next,
  .norm=ff_iter_fltp_norm,
};

///////////////////////////////////////////////////////////////////////////////
static void ff_iter_dblp_first(ff_iter_t *i)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch) {
#if defined (__GNUC__) // [
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif // ]
    i->p.dbl.rp[ch]=(double *)i->frame->data[ch];
#if defined (__GNUC__) // [
#pragma GCC diagnostic pop
#endif // ]
  }

  i->p.dbl.mp=i->p.dbl.rp[0]+i->frame->nb_samples;
}

static int ff_iter_dblp_valid(ff_iter_t *i)
{
  return i->p.dbl.rp[0]<i->p.dbl.mp;
}

static void ff_iter_dblp_next(ff_iter_t *i)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch)
    ++i->p.dbl.rp[ch];
}

static void ff_iter_dblp_norm(ff_iter_t *i, double *x, double *max)
{
  int ch;

  for (ch=0;ch<i->frame->channels;++ch)
    ff_iter_norm(i,x,max,i->frame->channels,ch,*i->p.dbl.rp[ch]);
}

static ff_iter_vmt_t ff_iter_dblp_vmt={
  .id="dblp",
  .first=ff_iter_dblp_first,
  .valid=ff_iter_dblp_valid,
  .next=ff_iter_dblp_next,
  .norm=ff_iter_dblp_norm,
};

///////////////////////////////////////////////////////////////////////////////
static void ff_iter_err_first(ff_iter_t *i FFUNUSED)
{
  _DMESSAGE("invalid frame iterator");
}

static int ff_iter_err_valid(ff_iter_t *i FFUNUSED)
{
  _DMESSAGE("invalid frame iterator");

  return 0;
}

static void ff_iter_err_next(ff_iter_t *i FFUNUSED)
{
  _DMESSAGE("invalid frame iterator");
}

static void ff_iter_err_norm(ff_iter_t *i FFUNUSED, double *x FFUNUSED,
    double *max FFUNUSED)
{
  _DMESSAGE("invalid frame iterator");
}

static ff_iter_vmt_t ff_iter_err_vmt={
  .id="err",
  .first=ff_iter_err_first,
  .valid=ff_iter_err_valid,
  .next=ff_iter_err_next,
  .norm=ff_iter_err_norm,
};

FF_ENABLE_DEPRECATION_WARNINGS // ]
