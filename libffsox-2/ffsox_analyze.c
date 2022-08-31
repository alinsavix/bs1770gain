/*
 * ffsox_analyze.c
 * Copyright (C) 2015 Peter Belkner <pbelkner@users.sf.net>
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
#include <ffsox_priv.h>
#if defined (_WIN32) // [
#include <fcntl.h>

#define W_WIN32
#endif // ]

#define FFSOX_ANALYZE_RATE 192000.0

#define FFSOX_ANALYZE_CASCADE
int ffsox_analyze(analyze_config_t *ac, int ai, int vi)
{
  int code=-1;
  aggregate_t *aggregate=ac->aggregate;
  source_cb_t progress=NULL==ac->f?NULL:ffsox_source_progress;
  source_t si;
  frame_reader_t *fr;
  collect_config_t cc;
  collect_t collect;
  intercept_t intercept;
  sox_reader_t *read;
  sox_signalinfo_t signal;
  sox_effects_chain_t *chain;
  int n;
  char *opts[2];
#if defined (FFSOX_ANALYZE_CASCADE) // {
  sox_rate_t rate;
#endif // }
  char buf[32];

//DMARKLN();
#if defined (W_WIN32) // [
  if (ac->utf16) {
    fflush(ac->f);
    _setmode(_fileno(ac->f),_O_TEXT);
  }
#endif // ]

  // create a source.
//DMARKLN();
  if (ffsox_source_create(&si,ac->path,ai,vi,progress,ac->f)<0) {
    DMESSAGE("creating source");
    goto si;
  }

//DMARKLN();
  if (0!=ac->dump&&NULL!=ac->f)
    av_dump_format(si.f.fc,0,si.f.path,0);

//DMARKLN();
  // create a frame reader.
  if (NULL==(fr=ffsox_frame_reader_new(&si,si.ai,ac->stereo,ac->drc))) {
    DMESSAGE("creating frame reader");
    goto fr;
  }

//DMARKLN();
  // create a BS.1770 collector.
  memset(&cc,0,sizeof cc);
  cc.aggregate=ac->aggregate;
  cc.scale=0.25;
  cc.samplerate=fr->si.cc->sample_rate;
  cc.channels=fr->si.cc->channels;
  cc.momentary=ac->momentary;
  cc.shortterm=ac->shortterm;
#if defined (FFSOX_LFE_CHANNEL) // [
  cc.lfe=ac->lfe;
#endif // ]

//DMARKLN();
  if (ffsox_collect_create(&collect,&cc)<0) {
    DMESSAGE("creating collector");
    goto collect;
  }

//DMARKLN();
  // create a SoX reader.
  intercept.data=&collect;
  intercept.channel=ffsox_collect_channel;
  intercept.sample=ffsox_collect_sample;

//DMARKLN();
  if (NULL==(read=ffsox_sox_reader_new(fr,collect.scale,&intercept))) {
    DMESSAGE("creating SoX reader");
    goto sa;
  }

//DMARKLN();
  signal=read->signal;

//DMARKLN();
  // create the SoX chain.
  if (NULL==(chain=sox_create_effects_chain(&read->encoding,NULL))) {
    DMESSAGE("creating SoX chain");
    goto chain;
  }

//DMARKLN();
  // add the SoX read effect to the SoX chain.
  n=0;
  opts[n++]=(char *)read;

//DMARKLN();
  if (SOX_SUCCESS!=ffsox_sox_add_effect_fn(chain,&signal,&signal,
      n,opts,ffsox_sox_read_handler)) {
    DMESSAGE("creating SoX read effect");
    goto effect;
  }

//DMARKLN();
  // add the SoX rate effect to the SoX chain.
  if (0!=(AGGREGATE_TRUEPEAK&aggregate->flags)) {
#if defined (FFSOX_ANALYZE_CASCADE) // {
//#error FFSOX_ANALYZE_CASCADE
    rate=signal.rate;

//DMARKLN();
    while ((rate*=2.0)<=FFSOX_ANALYZE_RATE) {
//DMARKLN();
      //rate*=2;
      n=0;
      sprintf(buf,"%.0f",rate);
      opts[n++]=buf;

      if (SOX_SUCCESS!=ffsox_sox_add_effect_name(chain,&signal,&signal,
          n,opts,"rate")) {
        DMESSAGE("creating SoX rate effect");
        goto effect;
      }
    }
#else // } {
//DMARKLN();
    n=0;
    sprintf(buf,"%.0f",FFSOX_ANALYZE_RATE);
    opts[n++]=buf;

//DMARKLN();
    if (SOX_SUCCESS!=ffsox_sox_add_effect_name(chain,&signal,&signal,
        n,opts,"rate")) {
      DMESSAGE("creating SoX rate effect");
      goto effect;
    }
#endif // }
  }

//DMARKLN();
  // add the SoX pull effect to the SoX chain.
  n=0;

//DMARKLN();
  if (0!=(AGGREGATE_TRUEPEAK&aggregate->flags)) {
    opts[n++]=(char *)ffsox_collect_truepeak;
    opts[n++]=(char *)&collect;
  }

//DMARKLN();
  if (SOX_SUCCESS!=ffsox_sox_add_effect_fn(chain,&signal,&signal,
      n,opts,ffsox_sox_pull_handler)) {
    DMESSAGE("creating SoX pull effect");
    goto effect;
  }

//DMARKLN();
  // run the SoX chain.
  if (SOX_SUCCESS!=sox_flow_effects(chain,NULL,NULL)
      ||0!=read->sox_errno) {
    DMESSAGE("running SoX effects chain");
    goto flow;
  }

//DMARKLN();
  ffsox_collect_flush(&collect);
//DMARKLN();
  code=0;
// cleanup:
flow:
effect:
//DMARKLN();
  sox_delete_effects_chain(chain);
chain:
sa:
//DMARKLN();
  ffsox_collect_cleanup(&collect);
collect:
fr:
//DMARKLN();
  si.vmt->cleanup(&si);
si:
//DMARKLN();
#if defined (W_WIN32) // [
  if (ac->utf16) {
    fflush(ac->f);
    _setmode(_fileno(ac->f),_O_WTEXT);
  }
#if 0 // [
mode:
#endif // ]
#endif // ]
//DMARKLN();
  return code;
}
