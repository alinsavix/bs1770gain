/*
 * bg_process.c
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
#include <bg.h>
#if defined (BG_PARAM_SCRIPT) // [
#if ! defined (_WIN32) // [
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#elif defined (BG_SYSTEM) // ] [
#include <sys/types.h>
#include <sys/stat.h>
#endif // ]

//#define BG_ENVIR_DEBUG

typedef const ffchar_t *ffenvir_t[2];

#if defined (_WIN32) // [
static size_t envira2envirp(const ffenvir_t *envira, ffchar_t *envirp)
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
    if ((*envira)[1]) {
      if (envirp) {
        wcscpy(envirp,(*envira)[0]);
        envirp+=wcslen(envirp)+1;
        envirp[-1]=L'=';
        wcscpy(envirp,(*envira)[1]);
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
#if defined (_WIN32) // [
static ffchar_t *bg_process_get_envirp(bg_tree_t *tree)
#else // ] [
static ffchar_t **bg_process_get_envirpp(bg_tree_t *tree)
#endif // ]
{
  ffchar_t norm[32];
  ffchar_t momentary_mean[32];
  ffchar_t momentary_mean_relative[32];
  ffchar_t momentary_maximum[32];
  ffchar_t momentary_maximum_relative[32];
  ffchar_t momentary_range[32];
  ffchar_t shortterm_mean[32];
  ffchar_t shortterm_mean_relative[32];
  ffchar_t shortterm_maximum[32];
  ffchar_t shortterm_maximum_relative[32];
  ffchar_t shortterm_range[32];
  ffchar_t samplepeak_absolute[32];
  ffchar_t samplepeak_relative[32];
  ffchar_t truepeak_absolute[32];
  ffchar_t truepeak_relative[32];

  const ffenvir_t envir[]={
    {FFL("BS1770GAIN_NORM"),norm},
    {FFL("BS1770GAIN_INTEGRATED"),momentary_mean},
    {FFL("BS1770GAIN_MOMENTARY_MEAN"),momentary_mean},
    {FFL("BS1770GAIN_MOMENTARY_MEAN_RELATIVE"),momentary_mean_relative},
    {FFL("BS1770GAIN_MOMENTARY_MAXIMUM"),momentary_maximum},
    {FFL("BS1770GAIN_MOMENTARY_MAXIMUM_RELATIVE"),momentary_maximum_relative},
    {FFL("BS1770GAIN_MOMENTARY_RANGE"),momentary_range},
    {FFL("BS1770GAIN_SHORTTERM_MEAN"),shortterm_mean},
    {FFL("BS1770GAIN_SHORTTERM_MEAN_RELATIVE"),shortterm_mean_relative},
    {FFL("BS1770GAIN_SHORTTERM_MAXIMUM"),shortterm_maximum},
    {FFL("BS1770GAIN_SHORTTERM_MAXIMUM_RELATIVE"),shortterm_maximum_relative},
    {FFL("BS1770GAIN_SHORTTERM_RANGE"),shortterm_range},
    {FFL("BS1770GAIN_SAMPLEPEAK_ABSOLUTE"),samplepeak_absolute},
    {FFL("BS1770GAIN_SAMPLEPEAK_RELATIVE"),samplepeak_relative},
    {FFL("BS1770GAIN_TRUEPEAK_ABSOLUTE"),truepeak_absolute},
    {FFL("BS1770GAIN_TRUEPEAK_RELATIVE"),truepeak_relative},
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
    {NULL,NULL}
  };

  size_t size;
#if defined (_WIN32) // [
  ffchar_t *envirp;
#else // ] [
  ffchar_t **envirpp;
#endif // ]

  _SNPRINTF(norm,(sizeof norm)/(sizeof norm[0]),"%0.2lf",
      bg_print_conf_norm(tree));

  if (BG_FLAGS_AGG_MOMENTARY_MEAN&tree->param->flags.aggregate) {
    _SNPRINTF(momentary_mean,
        (sizeof momentary_mean)/(sizeof momentary_mean[0]),
        "%0.2f",
      bg_print_conf_momentary_mean(tree));
    _SNPRINTF(momentary_mean_relative,
        (sizeof momentary_mean_relative)/(sizeof momentary_mean_relative[0]),
        "%0.2f",
        bg_print_conf_momentary_mean_relative(tree));
  }
  else {
    memset(momentary_mean,0,sizeof momentary_mean);
    memset(momentary_mean_relative,0,sizeof momentary_mean_relative);
  }

  if (BG_FLAGS_AGG_MOMENTARY_MAXIMUM&tree->param->flags.aggregate) {
    _SNPRINTF(momentary_maximum,
        (sizeof momentary_maximum)/(sizeof momentary_maximum[0]),
        "%0.2f",
        bg_print_conf_momentary_maximum(tree));
    _SNPRINTF(momentary_maximum_relative,
        (sizeof momentary_maximum_relative)
            /(sizeof momentary_maximum_relative[0]),
        "%0.2f",
        bg_print_conf_momentary_maximum_relative(tree));
  }
  else {
    memset(momentary_maximum,0,sizeof momentary_maximum);
    memset(momentary_maximum_relative,0,sizeof momentary_maximum_relative);
  }

  if (BG_FLAGS_AGG_MOMENTARY_RANGE&tree->param->flags.aggregate) {
    _SNPRINTF(momentary_range,
        (sizeof momentary_range)/(sizeof momentary_range[0]),
        "%0.2f",
        bg_print_conf_momentary_range(tree));
  }
  else
    memset(momentary_range,0,sizeof momentary_range);

  if (BG_FLAGS_AGG_SHORTTERM_MEAN&tree->param->flags.aggregate) {
    _SNPRINTF(shortterm_mean,
        (sizeof shortterm_mean)/(sizeof shortterm_mean[0]),
        "%0.2f",
        bg_print_conf_shortterm_mean(tree));
    _SNPRINTF(shortterm_mean_relative,
        (sizeof shortterm_mean_relative)/(sizeof shortterm_mean_relative[0]),
        "%0.2f",
        bg_print_conf_shortterm_mean_relative(tree));
  }
  else {
    memset(shortterm_mean,0,sizeof shortterm_mean);
    memset(shortterm_mean_relative,0,sizeof shortterm_mean_relative);
  }

  if (BG_FLAGS_AGG_SHORTTERM_MAXIMUM&tree->param->flags.aggregate) {
    _SNPRINTF(shortterm_maximum,
        (sizeof shortterm_maximum)/(sizeof shortterm_maximum[0]),
        "%0.2f",
        bg_print_conf_shortterm_maximum(tree));
    _SNPRINTF(shortterm_maximum_relative,
        (sizeof shortterm_maximum_relative)
            /(sizeof *shortterm_maximum_relative),
        "%0.2f",
        bg_print_conf_shortterm_maximum_relative(tree));
  }
  else {
    memset(shortterm_maximum,0,sizeof shortterm_maximum);
    memset(shortterm_maximum_relative,0,sizeof shortterm_maximum_relative);
  }

  if (BG_FLAGS_AGG_SHORTTERM_RANGE&tree->param->flags.aggregate) {
    _SNPRINTF(shortterm_range,
        (sizeof shortterm_range)/(sizeof shortterm_range[0]),
        "%0.2f",
        bg_print_conf_shortterm_range(tree));
  }
  else
    memset(shortterm_range,0,sizeof shortterm_range);

  if (BG_FLAGS_AGG_SAMPLEPEAK&tree->param->flags.aggregate) {
    _SNPRINTF(samplepeak_absolute,
        (sizeof samplepeak_absolute)/(sizeof samplepeak_absolute[0]),
        "%0.2f",
        bg_print_conf_samplepeak_absolute(tree));
    _SNPRINTF(samplepeak_relative,
        (sizeof samplepeak_relative)/(sizeof samplepeak_relative[0]),
        "%0.2f",
        bg_print_conf_samplepeak_relative(tree));
  }
  else {
    memset(samplepeak_absolute,0,sizeof samplepeak_absolute);
    memset(samplepeak_relative,0,sizeof samplepeak_relative);
  }

  if (BG_FLAGS_AGG_TRUEPEAK&tree->param->flags.aggregate) {
    _SNPRINTF(truepeak_absolute,
        (sizeof truepeak_absolute)/(sizeof truepeak_absolute[0]),
        "%0.2f",
        bg_print_conf_truepeak_absolute(tree));
    _SNPRINTF(truepeak_relative,
        (sizeof truepeak_relative)/(sizeof truepeak_relative[0]),
        "%0.2f",
        bg_print_conf_truepeak_relative(tree));
  }
  else {
    memset(truepeak_absolute,0,sizeof truepeak_absolute);
    memset(truepeak_relative,0,sizeof truepeak_relative);
  }

#if defined (_WIN32) // [
  size=envira2envirp(envir,NULL);

  if (!size||!(envirp=malloc(size))) {
    _DMESSAGE("allocating envirp");
    goto e_envirp_alloc;
  }

  if (envira2envirp(envir,envirp)!=size) {
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
#if 0 // [
  static const wchar_t CMD[]=L"/bin/sh -c";
#else // ] [
  static const wchar_t CMD_EXEC[]=L"C:\\Windows\\system32\\cmd.exe /C ";
  static const wchar_t CMD_CALL[]=L"C:\\Windows\\system32\\cmd.exe /C call ";
#endif // ]
#endif // ]

  int err=-1;
#if defined (_WIN32) // [
  struct _stat buf;
  const wchar_t *CMD;
  STARTUPINFOW startup_info;
  PROCESS_INFORMATION process_info;
  wchar_t *cmd,*wp;
  ffchar_t *envirp;
  int bRes;
#endif // ]

#if defined (_WIN32) // [
  memset(&startup_info,0,sizeof startup_info);
  startup_info.cb=sizeof startup_info;
  memset(&process_info,0,sizeof process_info);

  CMD=_wstat(tree->param->script,&buf)?CMD_EXEC:CMD_CALL;
  wp=cmd=malloc((wcslen(CMD)+wcslen(tree->param->script)+2)*sizeof *cmd);

  if (!CMD) {
    DMESSAGE("allocating cmd");
    goto e_cmd;
  }

  wcscpy(wp,CMD);
  wp+=wcslen(wp);
  *wp++=L'"';
  wcscpy(wp,tree->param->script);
  wp+=wcslen(wp);
  *wp++=L'"';
  *wp=L'\0';

  if (!(envirp=bg_process_get_envirp(tree))) {
    _DMESSAGE("creating envirp");
    goto e_envirp;
  }

#if defined (BG_ENVIR_DEBUG) // [
_DWRITELNV("envirp:\"%" PBU_PRIs "\"",envirp);
exit(1);
#endif // ]

  bRes=CreateProcessW(
    NULL,                       // LPCWSTR               lpApplicationName,
    cmd,                        // LPWSTR                lpCommandLine,
    NULL,                       // LPSECURITY_ATTRIBUTES lpProcessAttributes,
    NULL,                       // LPSECURITY_ATTRIBUTES lpThreadAttributes,
    TRUE,                       // BOOL                  bInheritHandles,
    CREATE_UNICODE_ENVIRONMENT, // DWORD                 dwCreationFlags,
    envirp,                     // LPVOID                lpEnvironment,
    NULL,                       // LPCWSTR               lpCurrentDirectory,
    &startup_info,              // LPSTARTUPINFOW        lpStartupInfo,
    &process_info               // LPPROCESS_INFORMATION lpProcessInformation
  );

  if (bRes) {
    WaitForSingleObject(process_info.hProcess,INFINITE);
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
    err=0;
  }
  else
    _DMESSAGE("creating process");
//cleanup:
  free(envirp);
e_envirp:
  free(cmd);
e_cmd:
#else // ] [
  pid_t pid=fork();

  if (pid<0) {
    // something went wrong.
    _DMESSAGE("forking");
  }
  else if (0<pid) {
    // we're still the bs1770gain process. we're waiting until the shell
    // process (cf. below) terminates.
    waitpid(pid,NULL,0);
    err=0;
  }
  else {
    // we're the forked process. we're going to start a shell.
    char *argv[]={"/bin/sh","-c",tree->param->script,NULL};
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
