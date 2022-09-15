/*
 * bg_process.c
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
#if defined (BG_PARAM_SCRIPT) // [
#if defined (_WIN32) // [
#include <sys/types.h>
#include <sys/stat.h>
#else // ] [
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif // ]

//#define BG_ENVIR_DEBUG

typedef const ffchar_t *ffenvir_t[2];

#if defined (_WIN32) // [
static void path2posix(ffchar_t *p)
{
  while ((p=wcsstr(p,L"\\")))
    *p=L'/';
}

static size_t envira2envirp(const ffenvir_t *envira, ffchar_t *envirp,
    int posix)
{

  wchar_t *envir=GetEnvironmentStringsW(),*rp=envir;

  size_t size=0;
  size_t size_loop;

  if (envir) {
    // for Windows, we need to clone the environment.
    while (*rp) {
      size_loop=wcslen(rp)+1;

      if (envirp) {
        wcscpy(envirp,rp);
        envirp+=size_loop;
#if defined (BG_ENVIR_DEBUG) // [
        envirp[-1]=L'#';
#endif // ]
      }

      rp+=size_loop;
      size+=size_loop*sizeof *envirp;
    }

    size+=rp-envir;
    FreeEnvironmentStringsW(envir);
  }

  while ((*envira)[0]) {
    if ((*envira)[1][0]) {
      if (envirp) {
        wcscpy(envirp,(*envira)[0]);
        envirp+=wcslen(envirp)+1;
        envirp[-1]=L'=';
        wcscpy(envirp,(*envira)[1]);

        if (posix)
          path2posix(envirp);

        envirp+=wcslen(envirp)+1;
#if defined (BG_ENVIR_DEBUG) // [
        envirp[-1]=L'#';
#endif // ]
      }

      size_loop=wcslen((*envira)[0])*sizeof *envirp;
      size_loop+=sizeof *envirp;
      size_loop=wcslen((*envira)[1])*sizeof *envirp;
      size_loop+=sizeof *envirp;

      size+=size_loop;
    }

    ++envira;
  }

  size+=sizeof *envirp;

  if (envirp)
    *envirp++=L'\0';

  return size;
}
#else // ] [
static size_t envira2envirpp(const ffenvir_t *envira, ffchar_t **envirpp)
{
  int size=0;
  const ffenvir_t *rp=envira;
  ffchar_t **wpp=envirpp;
  ffchar_t *wp;

  while ((*rp)[0]) {
    if ((*rp)[1]&&wpp)
      ++wpp;

    ++rp;
  }

  // we need to allocate space for a terminating NULL!!! [
  if (wpp)
    *wpp++=NULL;

  size+=sizeof wpp;
  // ]

  wp=(ffchar_t *)wpp;
  rp=envira;
  wpp=envirpp;

  while ((*rp)[0]) {
    if ((*rp)[1]) {
      if (wpp) {
        *wpp=wp;
        strcpy(wp,(*rp)[0]);
        wp+=strlen(wp);
        *wp++='=';
        strcpy(wp,(*rp)[1]);
        wp+=strlen(wp);
        ++wp;
        ++wpp;
      }

      size+=strlen((*rp)[0]);
      ++size;
      size+=strlen((*rp)[1]);
      ++size;

      size+=sizeof wpp;
    }

    ++rp;
  }

  return size;
}
#endif // ]

static int bg_process_muxer_track_annotation_create(bg_tree_t *tree)
{
  int err=-1;
#if defined (FF_FLAC_HACK) // [
  int hack;
#endif // ]

  if (tree->param->output.dirname||tree->param->overwrite) {
#if defined (FF_FLAC_HACK) // [
    if (ff_input_open_muxer(&tree->track.input,&hack)<0) {
      _DMESSAGE("re-opening input");
      goto e_input;
    }
#else // ] [
    if (ff_input_open_muxer(&track->input)<0) {
      _DMESSAGE("re-opening input");
      goto e_input;
    }
#endif // ]

    if (tree->vmt->annotation.create(tree)<0) {
      _DMESSAGE("annotating");
      goto e_annotate;
    }
  }

  err=0;
e_annotate:
  ff_input_close(&tree->track.input);
e_input:
  return err;
}

static void bg_process_muxer_track_annotation_destroy(bg_tree_t *tree)
{
  if (tree->param->output.dirname||tree->param->overwrite)
    tree->vmt->annotation.destroy(tree);
}

static void bg_process_muxer_track_comment(bg_tree_t *tree)
{
  bg_param_t *param=tree->param;
  bg_track_t *track=&tree->track;

#if defined (BG_PARAM_QUIET) // [
  if (!param->quiet) {
#endif // ]
#if defined (BG_PARAM_SKIP_SCAN) // [
    if (param->skip_scan)
      fprintf(stdout,"%s\n",bg_tree_out_basename(tree));
    else {
#endif // ]
      fprintf(stdout,"[%lu/%lu] %s\n",track->root.id,param->count.max,
          bg_tree_out_basename(tree));
#if defined (BG_PARAM_SKIP_SCAN) // [
    }
#endif // ]

    fflush(stdout);
#if defined (BG_PARAM_QUIET) // [
  }
#endif // ]
}

#define BG_ENVIR_ALBUM
#if defined (_WIN32) // [
static ffchar_t *bg_process_get_envirp(bg_tree_t *tree, int posix)
#else // ] [
static ffchar_t **bg_process_get_envirpp(bg_tree_t *tree)
#endif // ]
{
#if defined (BG_ENVIR_ALBUM) // [
  bg_tree_t *parent=tree->parent;
#endif // ]
  ffchar_t norm[32];
  /////////////////////////////////////////////////////////////////////////////
  ffchar_t track_momentary_mean[32];
  ffchar_t track_momentary_mean_relative[32];
  ffchar_t track_momentary_maximum[32];
  ffchar_t track_momentary_maximum_relative[32];
  ffchar_t track_momentary_range[32];
  ////////
  ffchar_t track_shortterm_mean[32];
  ffchar_t track_shortterm_mean_relative[32];
  ffchar_t track_shortterm_maximum[32];
  ffchar_t track_shortterm_maximum_relative[32];
  ffchar_t track_shortterm_range[32];
  /////////////////////////////////////////////////////////////////////////////
  ffchar_t track_samplepeak_absolute[32];
  ffchar_t track_samplepeak_relative[32];
  ffchar_t track_truepeak_absolute[32];
  ffchar_t track_truepeak_relative[32];
#if defined (BG_ENVIR_ALBUM) // [
  ////////
  ffchar_t album_momentary_mean[32];
  ffchar_t album_momentary_mean_relative[32];
  ffchar_t album_momentary_maximum[32];
  ffchar_t album_momentary_maximum_relative[32];
  ffchar_t album_momentary_range[32];
  ////////
  ffchar_t album_shortterm_mean[32];
  ffchar_t album_shortterm_mean_relative[32];
  ffchar_t album_shortterm_maximum[32];
  ffchar_t album_shortterm_maximum_relative[32];
  ffchar_t album_shortterm_range[32];
  /////////////////////////////////////////////////////////////////////////////
  ffchar_t album_samplepeak_absolute[32];
  ffchar_t album_samplepeak_relative[32];
  ffchar_t album_truepeak_absolute[32];
  ffchar_t album_truepeak_relative[32];
#endif // ]

  const ffenvir_t envir[]={
    {FFL("BS1770GAIN_SOURCE"),tree->source.path},
    {FFL("BS1770GAIN_TARGET"),tree->target.path},
    //{FFL("BS1770GAIN_TARGET_DIRECTORY"),tree->parent->target.path},
#if defined (_WIN32) // [
    {FFL("BS1770GAIN_LU"),bg_print_conf_unit_luw(tree)},
    {FFL("BS1770GAIN_LRA"),bg_print_conf_unit_lraw(tree)},
#else // ] [
    {FFL("BS1770GAIN_LU"),bg_print_conf_unit_lum(tree)},
    {FFL("BS1770GAIN_LRA"),bg_print_conf_unit_lram(tree)},
#endif // ]
    {FFL("BS1770GAIN_OVERWRITE"),tree->param->overwrite?FFL("yes"):FFL("")},
    {FFL("BS1770GAIN_NORM"),norm},
    ///////////////////////////////////////////////////////////////////////////
    {FFL("BS1770GAIN_INTEGRATED"),
        track_momentary_mean},
    {FFL("BS1770GAIN_INTEGRATED_RELATIVE"),
        track_momentary_mean_relative},
    {FFL("BS1770GAIN_MOMENTARY_MEAN"),
        track_momentary_mean},
    {FFL("BS1770GAIN_MOMENTARY_MEAN_RELATIVE"),
        track_momentary_mean_relative},
    {FFL("BS1770GAIN_MOMENTARY_MAXIMUM"),
        track_momentary_maximum},
    {FFL("BS1770GAIN_MOMENTARY_MAXIMUM_RELATIVE"),
        track_momentary_maximum_relative},
    {FFL("BS1770GAIN_MOMENTARY_RANGE"),
        track_momentary_range},
    ////////
    {FFL("BS1770GAIN_SHORTTERM"),
        track_shortterm_mean},
    {FFL("BS1770GAIN_SHORTTERM_RELATIVE"),
        track_shortterm_mean_relative},
    {FFL("BS1770GAIN_SHORTTERM_MEAN"),
        track_shortterm_mean},
    {FFL("BS1770GAIN_SHORTTERM_MEAN_RELATIVE"),
        track_shortterm_mean_relative},
    {FFL("BS1770GAIN_SHORTTERM_MAXIMUM"),
        track_shortterm_maximum},
    {FFL("BS1770GAIN_SHORTTERM_MAXIMUM_RELATIVE"),
        track_shortterm_maximum_relative},
    {FFL("BS1770GAIN_SHORTTERM_RANGE"),
        track_shortterm_range},
    ////////
    {FFL("BS1770GAIN_SAMPLEPEAK_ABSOLUTE"),
        track_samplepeak_absolute},
    {FFL("BS1770GAIN_SAMPLEPEAK_RELATIVE"),
        track_samplepeak_relative},
    {FFL("BS1770GAIN_TRUEPEAK_ABSOLUTE"),
        track_truepeak_absolute},
    {FFL("BS1770GAIN_TRUEPEAK_RELATIVE"),
        track_truepeak_relative},
    ///////////////////////////////////////////////////////////////////////////
    {FFL("BS1770GAIN_TRACK_INTEGRATED"),
        track_momentary_mean},
    {FFL("BS1770GAIN_TRACK_INTEGRATED_RELATIVE"),
        track_momentary_mean_relative},
    {FFL("BS1770GAIN_TRACK_MOMENTARY_MEAN"),
        track_momentary_mean},
    {FFL("BS1770GAIN_TRACK_MOMENTARY_MEAN_RELATIVE"),
        track_momentary_mean_relative},
    {FFL("BS1770GAIN_TRACK_MOMENTARY_MAXIMUM"),
        track_momentary_maximum},
    {FFL("BS1770GAIN_TRACK_MOMENTARY_MAXIMUM_RELATIVE"),
        track_momentary_maximum_relative},
    {FFL("BS1770GAIN_TRACK_MOMENTARY_RANGE"),
        track_momentary_range},
    ////////
    {FFL("BS1770GAIN_TRACK_SHORTTERM"),
        track_shortterm_mean},
    {FFL("BS1770GAIN_TRACK_SHORTTERM_RELATIVE"),
        track_shortterm_mean_relative},
    {FFL("BS1770GAIN_TRACK_SHORTTERM_MEAN"),
        track_shortterm_mean},
    {FFL("BS1770GAIN_TRACK_SHORTTERM_MEAN_RELATIVE"),
        track_shortterm_mean_relative},
    {FFL("BS1770GAIN_TRACK_SHORTTERM_MAXIMUM"),
        track_shortterm_maximum},
    {FFL("BS1770GAIN_TRACK_SHORTTERM_MAXIMUM_RELATIVE"),
        track_shortterm_maximum_relative},
    {FFL("BS1770GAIN_TRACK_SHORTTERM_RANGE"),
        track_shortterm_range},
    ////////
    {FFL("BS1770GAIN_TRACK_SAMPLEPEAK_ABSOLUTE"),
        track_samplepeak_absolute},
    {FFL("BS1770GAIN_TRACK_SAMPLEPEAK_RELATIVE"),
        track_samplepeak_relative},
    {FFL("BS1770GAIN_TRACK_TRUEPEAK_ABSOLUTE"),
        track_truepeak_absolute},
    {FFL("BS1770GAIN_TRACK_TRUEPEAK_RELATIVE"),
        track_truepeak_relative},
#if defined (BG_ENVIR_ALBUM) // [
    ///////////////////////////////////////////////////////////////////////////
    {FFL("BS1770GAIN_ALBUM_INTEGRATED"),
        album_momentary_mean},
    {FFL("BS1770GAIN_ALBUM_INTEGRATED_RELATIVE"),
        album_momentary_mean_relative},
    {FFL("BS1770GAIN_ALBUM_MOMENTARY_MEAN"),
        album_momentary_mean},
    {FFL("BS1770GAIN_ALBUM_MOMENTARY_MEAN_RELATIVE"),
        album_momentary_mean_relative},
    {FFL("BS1770GAIN_ALBUM_MOMENTARY_MAXIMUM"),
        album_momentary_maximum},
    {FFL("BS1770GAIN_ALBUM_MOMENTARY_MAXIMUM_RELATIVE"),
        album_momentary_maximum_relative},
    {FFL("BS1770GAIN_ALBUM_MOMENTARY_RANGE"),
        album_momentary_range},
    ////////
    {FFL("BS1770GAIN_ALBUM_SHORTTERM"),
        album_shortterm_mean},
    {FFL("BS1770GAIN_ALBUM_SHORTTERM_RELATIVE"),
        album_shortterm_mean_relative},
    {FFL("BS1770GAIN_ALBUM_SHORTTERM_MEAN"),
        album_shortterm_mean},
    {FFL("BS1770GAIN_ALBUM_SHORTTERM_MEAN_RELATIVE"),
        album_shortterm_mean_relative},
    {FFL("BS1770GAIN_ALBUM_SHORTTERM_MAXIMUM"),
        album_shortterm_maximum},
    {FFL("BS1770GAIN_ALBUM_SHORTTERM_MAXIMUM_RELATIVE"),
        album_shortterm_maximum_relative},
    {FFL("BS1770GAIN_ALBUM_SHORTTERM_RANGE"),
        album_shortterm_range},
    ////////
    {FFL("BS1770GAIN_ALBUM_SAMPLEPEAK_ABSOLUTE"),
        album_samplepeak_absolute},
    {FFL("BS1770GAIN_ALBUM_SAMPLEPEAK_RELATIVE"),
        album_samplepeak_relative},
    {FFL("BS1770GAIN_ALBUM_TRUEPEAK_ABSOLUTE"),
        album_truepeak_absolute},
    {FFL("BS1770GAIN_ALBUM_TRUEPEAK_RELATIVE"),
        album_truepeak_relative},
#endif // ]
    ///////////////////////////////////////////////////////////////////////////
    {NULL,NULL}
  };

  size_t size;
#if defined (_WIN32) // [
  ffchar_t *envirp;
#else // ] [
  ffchar_t **envirpp;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  _SNPRINTF(norm,(sizeof norm)/(sizeof norm[0]),"%0.2lf",
      bg_print_conf_norm(tree));

  /////////////////////////////////////////////////////////////////////////////
  if (BG_FLAGS_AGG_MOMENTARY_MEAN&tree->param->flags.aggregate) {
    size=(sizeof track_momentary_mean)/(sizeof track_momentary_mean[0]);
    _SNPRINTF(track_momentary_mean,size,"%0.2f",
      bg_print_conf_momentary_mean(tree));
    ////////
    size=(sizeof track_momentary_mean_relative)
        /(sizeof track_momentary_mean_relative[0]),
    _SNPRINTF(track_momentary_mean_relative,size,"%0.2f",
        bg_print_conf_momentary_mean_relative(tree));
#if defined (BG_ENVIR_ALBUM) // [
    ///////////////////////////////////////////////////////////////////////////
    size=(sizeof album_momentary_mean)/(sizeof album_momentary_mean[0]);
    _SNPRINTF(album_momentary_mean,size,"%0.2f",
      bg_print_conf_momentary_mean(parent));
    ////////
    size=(sizeof album_momentary_mean_relative)
        /(sizeof album_momentary_mean_relative[0]),
    _SNPRINTF(album_momentary_mean_relative,size,"%0.2f",
        bg_print_conf_momentary_mean_relative(parent));
#endif // ]
  }
  else {
    size=sizeof track_momentary_mean;
    memset(track_momentary_mean,0,size);
    ////////
    size=sizeof track_momentary_mean_relative;
    memset(track_momentary_mean_relative,0,size);
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=sizeof album_momentary_mean;
    memset(album_momentary_mean,0,size);
    ////////
    size=sizeof album_momentary_mean_relative;
    memset(album_momentary_mean_relative,0,size);
#endif // ]
  }

  if (BG_FLAGS_AGG_MOMENTARY_MAXIMUM&tree->param->flags.aggregate) {
    size=(sizeof track_momentary_maximum)
        /(sizeof track_momentary_maximum[0]);
    _SNPRINTF(track_momentary_maximum,size,"%0.2f",
        bg_print_conf_momentary_maximum(tree));
    ////////
    size=(sizeof track_momentary_maximum_relative)
        /(sizeof track_momentary_maximum_relative[0]),
    _SNPRINTF(track_momentary_maximum_relative,size,"%0.2f",
        bg_print_conf_momentary_maximum_relative(tree));
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=(sizeof album_momentary_maximum)
        /(sizeof album_momentary_maximum[0]);
    _SNPRINTF(album_momentary_maximum,size,"%0.2f",
        bg_print_conf_momentary_maximum(parent));
    ////////
    size=(sizeof album_momentary_maximum_relative)
        /(sizeof album_momentary_maximum_relative[0]),
    _SNPRINTF(album_momentary_maximum_relative,size,"%0.2f",
        bg_print_conf_momentary_maximum_relative(parent));
#endif // ]
  }
  else {
    size=sizeof track_momentary_maximum;
    memset(track_momentary_maximum,0,size);
    ////////
    size=sizeof track_momentary_maximum_relative;
    memset(track_momentary_maximum_relative,0,size);
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=sizeof album_momentary_maximum;
    memset(album_momentary_maximum,0,size);
    ////////
    size=sizeof album_momentary_maximum_relative;
    memset(album_momentary_maximum_relative,0,size);
#endif // ]
  }

  if (BG_FLAGS_AGG_MOMENTARY_RANGE&tree->param->flags.aggregate) {
    size=(sizeof track_momentary_range)/(sizeof track_momentary_range[0]);
    _SNPRINTF(track_momentary_range,size,"%0.2f",
        bg_print_conf_momentary_range(tree));
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=(sizeof album_momentary_range)/(sizeof album_momentary_range[0]);
    _SNPRINTF(album_momentary_range,size,"%0.2f",
        bg_print_conf_momentary_range(parent));
#endif // ]
  }
  else {
    size=sizeof track_momentary_range;
    memset(track_momentary_range,0,size);
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=sizeof album_momentary_range;
    memset(album_momentary_range,0,size);
#endif // ]
  }

  /////////////////////////////////////////////////////////////////////////////
  if (BG_FLAGS_AGG_SHORTTERM_MEAN&tree->param->flags.aggregate) {
    size=(sizeof track_shortterm_mean)/(sizeof track_shortterm_mean[0]);
    _SNPRINTF(track_shortterm_mean,size,"%0.2f",
        bg_print_conf_shortterm_mean(tree));
    ////////
    size=(sizeof track_shortterm_mean_relative)
        /(sizeof track_shortterm_mean_relative[0]),
    _SNPRINTF(track_shortterm_mean_relative,size,"%0.2f",
        bg_print_conf_shortterm_mean_relative(tree));
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=(sizeof album_shortterm_mean)/(sizeof album_shortterm_mean[0]);
    _SNPRINTF(album_shortterm_mean,size,"%0.2f",
        bg_print_conf_shortterm_mean(parent));
    ////////
    size=(sizeof album_shortterm_mean_relative)
        /(sizeof album_shortterm_mean_relative[0]),
    _SNPRINTF(album_shortterm_mean_relative,size,"%0.2f",
        bg_print_conf_shortterm_mean_relative(parent));
#endif // ]
  }
  else {
    size=sizeof track_shortterm_mean;
    memset(track_shortterm_mean,0,sizeof track_shortterm_mean);
    ////////
    size=sizeof track_shortterm_mean_relative;
    memset(track_shortterm_mean_relative,0,size);
  }

  if (BG_FLAGS_AGG_SHORTTERM_MAXIMUM&tree->param->flags.aggregate) {
    size=(sizeof track_shortterm_maximum)/(sizeof track_shortterm_maximum[0]),
    _SNPRINTF(track_shortterm_maximum,size,"%0.2f",
        bg_print_conf_shortterm_maximum(tree));
    ////////
    size=(sizeof track_shortterm_maximum_relative)
        /(sizeof *track_shortterm_maximum_relative);
    _SNPRINTF(track_shortterm_maximum_relative,size,"%0.2f",
        bg_print_conf_shortterm_maximum_relative(tree));
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=(sizeof album_shortterm_maximum)/(sizeof album_shortterm_maximum[0]),
    _SNPRINTF(album_shortterm_maximum,size,"%0.2f",
        bg_print_conf_shortterm_maximum(parent));
    ////////
    size=(sizeof album_shortterm_maximum_relative)
        /(sizeof *album_shortterm_maximum_relative);
    _SNPRINTF(album_shortterm_maximum_relative,size,"%0.2f",
        bg_print_conf_shortterm_maximum_relative(parent));
#endif // ]
  }
  else {
    size=sizeof track_shortterm_maximum;
    memset(track_shortterm_maximum,0,size);
    ////////
    size=sizeof track_shortterm_maximum_relative;
    memset(track_shortterm_maximum_relative,0,size);
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=sizeof album_shortterm_maximum;
    memset(album_shortterm_maximum,0,size);
    ////////
    size=sizeof album_shortterm_maximum_relative;
    memset(album_shortterm_maximum_relative,0,size);
#endif // ]
  }

  if (BG_FLAGS_AGG_SHORTTERM_RANGE&tree->param->flags.aggregate) {
    size=(sizeof track_shortterm_range)/(sizeof track_shortterm_range[0]);
    _SNPRINTF(track_shortterm_range,size,"%0.2f",
        bg_print_conf_shortterm_range(tree));
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=(sizeof album_shortterm_range)/(sizeof album_shortterm_range[0]);
    _SNPRINTF(album_shortterm_range,size,"%0.2f",
        bg_print_conf_shortterm_range(parent));
#endif // ]
  }
  else {
    size=sizeof track_shortterm_range;
    memset(track_shortterm_range,0,size);
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=sizeof album_shortterm_range;
    memset(album_shortterm_range,0,size);
#endif // ]
  }

  /////////////////////////////////////////////////////////////////////////////
  if (BG_FLAGS_AGG_SAMPLEPEAK&tree->param->flags.aggregate) {
    size=(sizeof track_samplepeak_absolute)
        /(sizeof track_samplepeak_absolute[0]);
    _SNPRINTF(track_samplepeak_absolute,size,"%0.2f",
        bg_print_conf_samplepeak_absolute(tree));
    ////////
    size=(sizeof track_samplepeak_relative)
        /(sizeof track_samplepeak_relative[0]);
    _SNPRINTF(track_samplepeak_relative,size,"%0.2f",
        bg_print_conf_samplepeak_relative(tree));
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=(sizeof album_samplepeak_absolute)
        /(sizeof album_samplepeak_absolute[0]);
    _SNPRINTF(album_samplepeak_absolute,size,"%0.2f",
        bg_print_conf_samplepeak_absolute(parent));
    ////////
    size=(sizeof album_samplepeak_relative)
        /(sizeof album_samplepeak_relative[0]);
    _SNPRINTF(album_samplepeak_relative,size,"%0.2f",
        bg_print_conf_samplepeak_relative(parent));
#endif // ]
  }
  else {
    size=sizeof track_samplepeak_absolute;
    memset(track_samplepeak_absolute,0,sizeof track_samplepeak_absolute);
    ////////
    size=sizeof track_samplepeak_relative;
    memset(track_samplepeak_relative,0,sizeof track_samplepeak_relative);
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=sizeof album_samplepeak_absolute;
    memset(album_samplepeak_absolute,0,sizeof album_samplepeak_absolute);
    ////////
    size=sizeof album_samplepeak_relative;
    memset(album_samplepeak_relative,0,sizeof album_samplepeak_relative);
#endif // ]
  }

  if (BG_FLAGS_AGG_TRUEPEAK&tree->param->flags.aggregate) {
    size=(sizeof track_truepeak_absolute)
        /(sizeof track_truepeak_absolute[0]);
    _SNPRINTF(track_truepeak_absolute,size,"%0.2f",
        bg_print_conf_truepeak_absolute(tree));
    ////////
    size=(sizeof track_truepeak_relative)
        /(sizeof track_truepeak_relative[0]);
    _SNPRINTF(track_truepeak_relative,size,"%0.2f",
        bg_print_conf_truepeak_relative(tree));
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=(sizeof album_truepeak_absolute)
        /(sizeof album_truepeak_absolute[0]);
    _SNPRINTF(album_truepeak_absolute,size,"%0.2f",
        bg_print_conf_truepeak_absolute(parent));
    ////////
    size=(sizeof album_truepeak_relative)
        /(sizeof album_truepeak_relative[0]);
    _SNPRINTF(album_truepeak_relative,size,"%0.2f",
        bg_print_conf_truepeak_relative(parent));
#endif // ]
  }
  else {
    size=sizeof track_truepeak_absolute;
    memset(track_truepeak_absolute,0,size);
    ////////
    size=sizeof track_truepeak_relative;
    memset(track_truepeak_relative,0,size);
#if defined (BG_ENVIR_ALBUM) // [
    ////////
    size=sizeof album_truepeak_absolute;
    memset(album_truepeak_absolute,0,size);
    ////////
    size=sizeof album_truepeak_relative;
    memset(album_truepeak_relative,0,size);
#endif // ]
  }

#if defined (_WIN32) // [
  size=envira2envirp(envir,NULL,posix);

  if (!size||!(envirp=malloc(size))) {
    _DMESSAGE("allocating envirp");
    goto e_envirp_alloc;
  }

  if (envira2envirp(envir,envirp,posix)!=size) {
    _DMESSAGE("initilizing envirp");
    goto e_envirp_init;
  }

  return envirp;
e_envirp_init:
  free(envirp);
e_envirp_alloc:
#else // ] [
  size=envira2envirpp(envir,NULL);

  if (!size||!(envirpp=malloc(size))) {
    _DMESSAGE("allocating envirpp");
    goto e_envirpp_alloc;
  }

  if (envira2envirpp(envir,envirpp)!=size) {
    _DMESSAGE("initilizing envirpp");
    goto e_envirpp_init;
  }

  return envirpp;
e_envirpp_init:
  free(envirpp);
e_envirpp_alloc:
#endif // ]
  return NULL;
}

static int _bg_process_tree_run_script(bg_tree_t *tree)
{
#if defined (_WIN32) // [
  static const wchar_t CMD_EXEC[]=L"C:\\Windows\\system32\\cmd.exe /C ";
  static const wchar_t CMD_CALL[]=L"C:\\Windows\\system32\\cmd.exe /C call ";
#endif // ]

  int err=-1;
  bg_param_t *param=tree->param;
#if defined (_WIN32) // [
  struct _stat buf;
  const wchar_t *CMD;
  STARTUPINFOW startup_info;
  PROCESS_INFORMATION process_info;
  wchar_t *cmd,*wp;
  ffchar_t *envirp=NULL;
  int bRes;
#if defined (BG_PARAM_SHELL) // [
  int posix;
#endif // ]

  memset(&startup_info,0,sizeof startup_info);
  startup_info.cb=sizeof startup_info;
  memset(&process_info,0,sizeof process_info);

#if defined (BG_PARAM_SHELL) // [
  if (param->shell.interpreter) {
    posix=1;
    wp=cmd=malloc((wcslen(param->shell.interpreter)
        +1
        +wcslen(param->shell.parameter)
        +1
        +1+wcslen(param->script)+1)*sizeof *cmd);

    if (!cmd) {
      DMESSAGE("allocating cmd");
      goto e_cmd;
    }

    wcscpy(wp,param->shell.interpreter);
    wp+=wcslen(wp);
    *wp++=L' ';
    wcscpy(wp,param->shell.parameter);
    wp+=wcslen(wp);
    *wp++=L' ';
    *wp++=L'"';
    wcscpy(wp,param->script);
    path2posix(wp);
    wp+=wcslen(wp);
    *wp++=L'"';
    *wp=L'\0';
  }
  else {
#endif // ]
    posix=0;
    CMD=_wstat(param->script,&buf)?CMD_EXEC:CMD_CALL;
    wp=cmd=malloc((wcslen(CMD)+wcslen(param->script)+2)*sizeof *cmd);

    if (!cmd) {
      DMESSAGE("allocating cmd");
      goto e_cmd;
    }

    wcscpy(wp,CMD);
    wp+=wcslen(wp);
    *wp++=L'"';
    wcscpy(wp,param->script);
    wp+=wcslen(wp);
    *wp++=L'"';
    *wp=L'\0';
#if defined (BG_PARAM_SHELL) // [
  }
#endif // ]

  if (!(envirp=bg_process_get_envirp(tree,posix))) {
    _DMESSAGE("creating envirp");
    goto e_envirp;
  }

#if defined (BG_ENVIR_DEBUG) // [
_DWRITELNV("envirp:\"%" PBU_PRIs "\"",envirp);
exit(1);
#endif // ]

  for (;;) {
    bRes=CreateProcessW(
      NULL,           // LPCWSTR               lpApplicationName,
      cmd,            // LPWSTR                lpCommandLine,
      NULL,           // LPSECURITY_ATTRIBUTES lpProcessAttributes,
      NULL,           // LPSECURITY_ATTRIBUTES lpThreadAttributes,
      TRUE,           // BOOL                  bInheritHandles,
      CREATE_UNICODE_ENVIRONMENT,
                      // DWORD                 dwCreationFlags,
      envirp,         // LPVOID                lpEnvironment,
      NULL,           // LPCWSTR               lpCurrentDirectory,
      &startup_info,  // LPSTARTUPINFOW        lpStartupInfo,
      &process_info   // LPPROCESS_INFORMATION lpProcessInformation
    );

    if (bRes) {
      DWORD dwExitCode;

      WaitForSingleObject(process_info.hProcess,INFINITE);
      GetExitCodeProcess(process_info.hProcess,&dwExitCode);
//_WRITELNV("dwExitCode:%lu",dwExitCode);
      CloseHandle(process_info.hProcess);
      CloseHandle(process_info.hThread);
      bg_process_muxer_track_comment(tree);
      free(envirp);

      if (!dwExitCode) {
        err=0;
        break;
      }
    }
    else {
      _DMESSAGE("creating process");
      free(envirp);
      break;
    }
//cleanup:
e_envirp:
  free(cmd);
e_cmd:
#else // ] [
  for (;;) {
    pid_t pid=fork();

    if (pid<0) {
      // something went wrong.
      _DMESSAGE("forking");
      break;
    }
    else if (0<pid) {
      // we're still the bs1770gain process. we're waiting until the shell
      // process (cf. below) terminates.
      int status;

      waitpid(pid,&status,0);
      bg_process_muxer_track_comment(tree);
//_WRITELNV("status:%d",status);
      if (!status) {
        err=0;
        break;
      }
  }
  else {
      // we're the forked process. we're going to start a shell.
#if defined (BG_PARAM_SHELL) // [
      char *argv[]={param->shell.interpreter,param->shell.parameter,
          param->script,NULL};
#else // ] [
      char *argv[]={"/bin/sh","-c",param->script,NULL};
#endif // ]
      ffchar_t **envirpp;

      if (!(envirpp=bg_process_get_envirpp(tree))) {
        _DMESSAGE("creating envirpp");
        goto e_envirpp;
      }

      err=execve(argv[0],argv,envirpp);
      // we should never go here ...
      free(envirpp);
    e_envirpp:
      _exit(EXIT_FAILURE);
    }
  }
#endif // ]
#if defined (_WIN32) // [
  }
#endif // ]

  return err;
}

int bg_process_tree_run_script(bg_tree_t *tree)
{
  int err=-1;

  if (!tree->param->script) {
    _DMESSAGE("missing script");
    goto e_script;
  }

  if (BG_TREE_TYPE_TRACK!=tree->vmt->type) {
    _DMESSAGEV("attempt to run script on \"%s\"",tree->vmt->id);
    goto e_type;
  }

  if (tree->param->script) {
    if (bg_process_muxer_track_annotation_create(tree)<0) {
      _DMESSAGE("annotating");
      goto e_annotate;
    }

    err=_bg_process_tree_run_script(tree);
    bg_process_muxer_track_annotation_destroy(tree);
e_annotate: ;
  }
e_type:
e_script:
  return err;
}
#endif // ]
