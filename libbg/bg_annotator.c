/*
 * bg_annotator.c
 *


 *
 * This program is PBU_FREE_GUARDED software; you can redistribute it and/or
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
#if defined (_WIN32) // [
#include <locale.h>
#else // ] [
#include <ctype.h>
#include <wctype.h>
#include <wchar.h>
#include <fcntl.h>
#include <unistd.h>
#endif // ]
#include <bg.h>

///////////////////////////////////////////////////////////////////////////////
static bg_tree_t *bg_tree_annotation_parent(bg_tree_t *tree)
{
  bg_param_t *param=tree->param;

  return tree->argv->lift<tree->depth?tree->parent
      :tree->argv->lift==tree->depth?&param->root:NULL;
}

///////////////////////////////////////////////////////////////////////////////
int bg_file_annotation_create(bg_tree_t *tree)
{
  const bg_param_t *param=tree->param;
  bg_tree_t *parent=bg_tree_annotation_parent(tree);
  size_t len1,len2;
  ssize_t size;
  ffchar_t *tp;

  /////////////////////////////////////////////////////////////////////////////
  len1=parent&&parent->target.path?FFSTRLEN(parent->target.path):0u;
  len2=tree->source.basename?FFSTRLEN(tree->source.basename):0u;
  size=(len1?len1+1u:0u)+(len2?len2+1u:0u);
  tree->target.path=tp=malloc(size*sizeof tp[0]);

  if (!tree->target.path) {
    _DMESSAGE("allocating output path");
    goto e_path;
  }

  if (parent&&parent->target.path) {
    FFSTRCPY(tp,parent->target.path);
    tp+=len1;

    if (tree->source.basename)
      *tp++=FFPATHSEP;
  }

  tree->target.basename=tp;

  if (tree->source.basename) {
    FFSTRCPY(tp,tree->source.basename);
    tp+=len2;
  }
  else
    *tp=0;

  /////////////////////////////////////////////////////////////////////////////
  len2=FFSTRLEN(param->temp_prefix);
  tree->temp.path=tp=malloc((size+len2)*sizeof tp[0]);

  if (!tree->temp.path) {
    _DMESSAGE("allocating temporary path");
    goto e_temp;
  }

  if (parent&&parent->target.path) {
    FFSTRCPY(tp,parent->target.path);
    tp+=len1;

    if (tree->source.basename)
      *tp++=FFPATHSEP;
  }

  tree->temp.basename=tp;
  FFSTRCPY(tp,param->temp_prefix);
  tp+=len2;
  FFSTRCPY(tp,tree->target.basename);

  if (ff_fexists(tree->temp.path)) {
#if defined (_WIN32) // [
    _DMESSAGEV("file \"%S\" exists; either remove it or define another"
        " prefix for temporary files by means of option --temp-prefix",
        tree->temp.path);
#else // ] [
    _DMESSAGEV("file \"%s\" exists; either remove it or define another"
        " prefix for temporary files by means of option --temp-prefix",
        tree->temp.path);
#endif // ]
    goto e_temp_exists;
  }

  /////////////////////////////////////////////////////////////////////////////
#if 0 // [
success:
#endif // ]
  return 0;
//cleanup:
e_temp_exists:
  PBU_FREE_GUARDED(tree->temp.path);
e_temp:
  PBU_FREE_GUARDED(tree->target.path);
e_path:
  return -1;
}

void bg_file_annotation_destroy(bg_tree_t *tree FFUNUSED)
{
  PBU_FREE_GUARDED(tree->temp.path);
  PBU_FREE_GUARDED(tree->target.path);
}

///////////////////////////////////////////////////////////////////////////////
#define BG_TRACK_AUDIO_SFX
#if defined (BG_TRACK_AUDIO_SFX) // [
static const ffchar_t *bg_track_audio_sfx(const bg_track_t *track FFUNUSED,
    const bg_tree_t *tree, const ffchar_t *default_sfx)
{
  const ffchar_t *sp,*sfx;

  // get the suffix from the input.
  sfx=sp=FFSTRSTR(tree->source.path,FFL("."));

  // find the right-most dot '.'.
  while (sp) {
    sfx=++sp;
    sp=FFSTRSTR(sp,FFL("."));
  }

  if (!sfx)
    return default_sfx;
  else if (track->input.audio.ctx) {
    if (!FFSTRCMP(FFL("flac"),sfx))
      return sfx;
    else
      return default_sfx;
  }
  else {
    if (!FFSTRCMP(FFL("flac"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("wv"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("mp3"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("ogg"),sfx))
      return sfx;
#if 0 // [
    else if (!FFSTRCMP(FFL("m4a"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("aac"),sfx))
      return sfx;
#endif // ]
    else
      return default_sfx;
  }
}
#endif // ]

#define BG_ALBUM_ART
static const ffchar_t *bg_track_out_sfx(const bg_track_t *track,
    const bg_tree_t *tree)
{
  const bg_param_t *param=tree->param;
#if defined (BG_ALBUM_ART) // [
  AVStream *vstream FFUNUSED=0<track->input.vi?
      track->input.fmt.ctx->streams[track->input.vi]:NULL;
#endif // ]

#if defined (BG_TRACK_AUDIO_SFX) // [
  if (param->out.sfx&&*param->out.sfx) {
    // in case a sfx is provided choose it.
    return param->out.sfx;
  }
  else {
    return bg_track_audio_sfx(track,tree,
        0<=track->input.vi?FFL("mkv"):FFL("mka"));
  }
#else // ] [
  if ((BG_FLAGS_MODE_APPLY&param->flags.mode)
      &&param->out.sfx&&*param->out.sfx) {
    // in case a sfx is provided choose it.
    return param->out.sfx;
  }
#if defined (BG_ALBUM_ART) // [
  else if (vstream&&!(vstream->disposition&AV_DISPOSITION_ATTACHED_PIC)) {
#else // ] [
  else if (0<=track->input.vi) {
#endif // ]
    // in case of a video:
    if (track->input.audio.ctx) {
      // when transcoding we choose MKV as the container.
#if defined (BG_TRACK_AUDIO_SFX) // [
      return bg_track_audio_sfx(track,tree,FFL("mkv"));
#else // ] [
      return FFL("mkv");
#endif // ]
    }
    else {
      // we always remux to mkv.
      return FFL("mkv");
    }
#if ! defined (BG_ALBUM_ART) // [
  }
#else // ] [
  }
#endif // ]
  else if (track->input.audio.ctx) {
    // we always transcode to FLAC.
    return FFL("flac");
  }
  else {
#if defined (BG_TRACK_AUDIO_SFX) // [
    return bg_track_audio_sfx(track,tree,FFL("mka"));
#else // ] [
    // get the suffix from the input.
    sfx=sp=FFSTRSTR(tree->source.path,FFL("."));

    // find the right-most dot '.'.
    while (sp) {
      sfx=++sp;
      sp=FFSTRSTR(sp,FFL("."));
    }

    if (!sfx)
      return FFL("mka");
    else if (!FFSTRCMP(FFL("flac"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("wv"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("mp3"),sfx))
      return sfx;
    else if (!FFSTRCMP(FFL("ogg"),sfx))
      return sfx;
    else
      return FFL("mka");
#endif // ]
  }
#endif // ]
}

#define BG_TRACK_BASENAME_TRACK
#if 0 // [
static int bg_track_basename(bg_track_t *track, bg_tree_t *tree,
    ffchar_t **opath, size_t len)
{
#if defined (_WIN32) && defined (BG_TRACK_BASENAME_TRACK) // [
  enum { TRACK_SIZE=64 };
#endif // ]
  bg_param_t *param=tree->param;
  AVDictionary *metadata=track->input.fmt.ctx->metadata;

  struct {
    const AVDictionaryEntry *title;
#if defined (BG_TRACK_BASENAME_TRACK) // [
    const AVDictionaryEntry *track;
#endif // ]
  } tag={
#if 0 // [
    .title=av_dict_get(metadata,"TITLE",NULL,AV_DICT_IGNORE_SUFFIX),
#if defined (BG_TRACK_BASENAME_TRACK) // [
    .track=av_dict_get(metadata,"TRACK",NULL,AV_DICT_IGNORE_SUFFIX),
#endif // ]
#else // ] [
    .title=av_dict_get(metadata,"TITLE",NULL,0),
#if defined (BG_TRACK_BASENAME_TRACK) // [
    .track=av_dict_get(metadata,"TRACK",NULL,0),
#endif // ]
#endif // ]
  };

#if defined (BG_TRACK_ID) // [
  int id=tag.track?atoi(tag.track->value):track->album.id;
#else // ] [
  int id=tag.track?atoi(tag.track->value):-1;
#endif // ]
#if defined (_WIN32) && ! defined (BG_WIN32_CREATE_LOCALE) // [
  const char *locale=NULL;
#endif // ]
  // NOTE: under Linux sizeof(wchar_t) is 4 bytes and under Windows
  //   it is 2 bytes.
  wchar_t *op;
  const ffchar_t *ipath,*sfx,*sp;

  if ((BG_FLAGS_EXT_RENAME&param->flags.extension)&&tag.title) {
#if defined (_WIN32) // [
    if (opath) {
      // swprintf() in any case writes a trailing '\0', hence (1+len).
#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (0<id)
        len=swprintf(*opath,1+track->target.pfx.len,L"%02d_",id);
      else
        len=0;
#else // ] [
      swprintf(*opath,1+track->target.pfx.len,
          track->album.id<100?L"%02lu_":L"%lu_",track->album.id);
#endif // ]
      wcscpy(*opath+track->target.pfx.len,track->target.title);
      PBU_FREE_GUARDED(track->target.title);
      track->target.title=NULL;
    }
    else {
      // claculate the length of the intermediate representation.
      len=MultiByteToWideChar(
        CP_UTF8,        // UINT                              CodePage,
        0ul,            // DWORD                             dwFlags,
        tag.title->value,
                        // _In_NLS_string_(cbMultiByte)LPCCH lpMultiByteStr,
        strlen(tag.title->value),
                        // int                               cbMultiByte,
        NULL,           // LPWSTR                            lpWideCharStr,
        0               // int                               cchWideChar
      );

      // allocate sufficient memory to hold the intermediate representatiion.
      track->target.title=malloc((len+1)*sizeof *track->target.title);

      if (!track->target.title) {
        _DMESSAGE("allocating intermediate path");
        // set error.
        len=-1;
        goto e_path;
      }

      // transform into the intermediate representation.
      MultiByteToWideChar(
        CP_UTF8,        // UINT                              CodePage,
        0ul,            // DWORD                             dwFlags,
        tag.title->value,
                        // _In_NLS_string_(cbMultiByte)LPCCH lpMultiByteStr,
        strlen(tag.title->value),
                        // int                               cbMultiByte,
        track->target.title,
                        // LPWSTR                            lpWideCharStr,
        len             // int                               cchWideChar
      );

      // add a trailing '\0' to the intermediate representation.
      track->target.title[len]=L'\0';
      // do some transformations, i.e. replace some characters by '_'
      // and transform to lower case.
      op=track->target.title;
#if ! defined (BG_WIN32_CREATE_LOCALE) // [
      // we need to temporarily switch locale in order that towlower() works
      // as expected because unfortunately on our system _create_locale()
      // isn't availabe and hence we have to rely on towlower().
      locale=setlocale(LC_ALL,NULL);
      setlocale(LC_ALL,"");
#endif // ]

      while (*op) {
        switch (*op) {
        case L'.':
        case L',':
        case L'/':
        case L'\\':
        case L'(':
        case L')':
        case L'[':
        case L']':
        case L'&':
        case L':':
        case L' ':
        case L'?':
        case L'\'':
          if (track->target.title<op&&'_'==op[-1])
            memmove(op,op+1,(wcslen(op+1)+1)*sizeof *op);
          else {
            *op=L'_';
            op=CharNextW(op);
          }

          break;
        default:
#if defined (BG_WIN32_CREATE_LOCALE) // [
          *op=_towlower_l(*op,tree->param->locale);
#else // ] [
          // our system _create_locale() isn't availabe.
          *op=towlower(*op);
#endif // ]
          op=CharNextW(op);
          break;
        }
      }

      // if necessary shorten the intermediate representation.
      if (track->target.title<op&&'_'==op[-1])
        op[-1]=L'\0';

#if ! defined (BG_WIN32_CREATE_LOCALE) // [
      // we need to switch back locale.
      setlocale(LC_ALL,locale);
#endif // ]

#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (0<id)
        len=swprintf(NULL,0,L"%02d_",id);
      else
        len=0;
#else // ] [
      len=swprintf(NULL,0,
          track->album.id<100?L"%02lu_":L"%lu_",track->album.id);
#endif // ]
      track->target.pfx.len=len;
      len+=wcslen(track->target.title);
    }
#else // ] [
    if (opath) {
      // snprintf() in any case writes a trailing '\0', hence (1+len).
      // I.e. we assume enough memory is allocated for holding a
      // trailing '\0'.
#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (0<id)
        snprintf(*opath,1+track->target.pfx.len,"%02d_",id);
#else // ] [
      snprintf(*opath,1+track->target.pfx.len,
          track->album.id<100?"%02lu_":"%lu_",track->album.id);
#endif // ]
      // we convert the intermediate representation back into utf-8.
      wcstombs(*opath+track->target.pfx.len,track->target.title,
          len-track->target.pfx.len);
      PBU_FREE_GUARDED(track->target.title);
      track->target.title=NULL;
    }
    else {
      // calculate the length for holding the intermediate representation.
      len=mbstowcs(NULL,tag.title->value,0);
      // allocate sufficient memory  in order to hold the intermediate
      // representation.
      track->target.title=malloc((len+1)*sizeof track->target.title[0]);

      if (!track->target.title) {
        _DMESSAGE("allocating intermediate path");
        // set error.
        len=-1;
        goto e_path;
      }

      // convert the title tag into the intermediate representation
      // including a trailing '\0'.
      mbstowcs(track->target.title,tag.title->value,len+1);

      // do some transformations.
      op=track->target.title;

      while (*op) {
        switch (*op) {
        case L'.':
        case L',':
        case L'/':
        case L'\\':
        case L'(':
        case L')':
        case L'[':
        case L']':
        case L'&':
        case L':':
        case L' ':
        case L'?':
        case L'\'':
          if (track->target.title<op&&'_'==op[-1])
            memmove(op,op+1,(wcslen(op+1)+1)*sizeof *op);
          else {
            *op='_';
            ++op;
          }

          break;
        default:
          *op=towlower(*op);
          ++op;
          break;
        }
      }

      if (track->target.title<op&&'_'==op[-1])
        op[-1]=L'\0';

#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (0<id)
        len=snprintf(NULL,0,"%02d_",id);
      else
        len=0;
#else // ] [
      len=snprintf(NULL,0,track->album.id<100?"%02lu_":"%lu_",track->album.id);
#endif // ]
      track->target.pfx.len=len;
      // add the length for holding the title (transformed to lower case.)
      len+=wcstombs(NULL,track->target.title,0);
    }
#endif // ]
  }
  else {
    if (tree->source.basename) {
      ipath=tree->source.basename;

      if (opath)
        memcpy(*opath,ipath,len*sizeof **opath);
      else {
        sfx=NULL;
        sp=FFSTRSTR(ipath,FFL("."));

        while (sp) {
          sfx=sp;
          sp=FFSTRSTR(++sp,FFL("."));
        }

        len=sfx?(size_t)(sfx-ipath):FFSTRLEN(ipath);
      }
    }
    else
      len=0;
  }
e_path:
  return len;
}
#else // ] [
#if defined (BG_UTF8_ITER) // [
#if ! defined (_WIN32) // [
#define BG_TITLE_NEXT_UTF8_ITER
#endif // ]
// TODO: consider
//   https://www.gnu.org/software/libunistring/manual/libunistring.html
#endif // ]
#if 0 // [
static int bg_title_next(const char *title, int special)
{
  int len=0;
#if defined (BG_TITLE_NEXT_UTF8_ITER) // [
  const char *next;
  size_t size;
  bg_utf8_iter_t i;
#elif defined (_WIN32) // ] [
  const char *next;
#endif // ]

#if ! defined (BG_TITLE_NEXT_UTF8_ITER) // ] [
  while (*title) {
#else // ] [
  size=0;
  bg_utf8_iter_first(&i,title,NULL,&size);

  while (i.vmt->valid(&i)) {
    i.vmt->apply(&i);

    if (1==size) {
#endif // ]
      switch (*title) {
      case '.':
      case ',':
      case '/':
      case '\\':
      case '(':
      case ')':
      case '[':
      case ']':
      case '&':
      case ':':
      case ' ':
      case '?':
      case '\'':
        if (special)
          return len;

        break;
      default:
        if (!special)
          return len;

        break;
      }
#if defined (BG_TITLE_NEXT_UTF8_ITER) // [
    }
    else if (!special)
      return len;

    size=0;
    next=(const char *)i.vmt->next(&i);
    len+=next-title;
    title=next;
  }
#else // ] [
#if defined (_WIN32) // [
    next=CharNextA(title);
    len+=next-title;
    title=next;
#else // ] [
    ++len;
    ++title;
#endif // ]
  }
#endif // ]

  return len;
}
#else // ] [
static int bg_title_next(const char *title, int special)
{
  int len=0;
  const char *next;

  while (*title) {
    next=bg_char_nexta(title);

    if (1==next-title) {
      switch (*title) {
      case '.':
      case ',':
      case '/':
      case '\\':
      case '(':
      case ')':
      case '[':
      case ']':
      case '&':
      case ':':
      case ' ':
      case '?':
      case '\'':
        if (special)
          return len;

        break;
      default:
        if (!special)
          return len;

        break;
      }
    }
    else if (!special)
      return len;

    len+=next-title;
    title=next;
  }

  return len;
}
#endif // ]

static int bg_title_length(char *title, wchar_t *wtitle, size_t leni)
{
  wchar_t *op=wtitle;
  int leno=0;
  int next,ch,len;

  for (;;) {
    if (!*title)
      break;

    ///////////////////////////////////////////////////////////////////////////
    next=bg_title_next(title,0);
    title+=next;

    if (*title) {
      if (next) {
        if (wtitle) {
          *wtitle++=L'_';
          *wtitle=L'\0';
          --leni;
        }

        ++leno;
      }
    }
    else
      return leno;

    ///////////////////////////////////////////////////////////////////////////
    next=bg_title_next(title,1);
    ch=title[next];
    title[next]=0;

#if defined (_WIN32) // [
    len=MultiByteToWideChar(
      CP_UTF8,        // UINT                              CodePage,
      0ul,            // DWORD                             dwFlags,
      title,          // _In_NLS_string_(cbMultiByte)LPCCH lpMultiByteStr,
      next,           // int                               cbMultiByte,
      wtitle,         // LPWSTR                            lpWideCharStr,
      leni            // int                               cchWideChar
    );
#else // ] [
    len=mbstowcs(wtitle,title,leni);
#endif // ]

    if (wtitle) {
      wtitle+=len;
      *wtitle=L'\0';
      leni-=len;
    }

    leno+=len;
    title+=next;
    *title=ch;
  }

  if (op) {
#if defined (_WIN32) // [
    _wcslwr(op);
#else // ] [
    while (*op) {
#if 0 && defined (_WIN32) // [
#if defined (BG_WIN32_CREATE_LOCALE) // [
      *op=_towlower_l(*op,tree->param->locale);
#else // ] [
          // our system _create_locale() isn't availabe.
      *op=towlower(*op);
#endif // ]
      op=CharNextW(op);
#else // ] [
      *op=towlower(*op);
      ++op;
#endif // ]
    }
#endif // ]
  }

  return leno;
}

static int bg_track_basename(bg_track_t *track, bg_tree_t *tree,
    ffchar_t **opath, size_t len)
{
#if defined (_WIN32) && defined (BG_TRACK_BASENAME_TRACK) // [
  enum { TRACK_SIZE=64 };
#endif // ]
  bg_param_t *param=tree->param;
  AVDictionary *metadata=track->input.fmt.ctx->metadata;

  struct {
    const AVDictionaryEntry *title;
#if defined (BG_TRACK_BASENAME_TRACK) // [
    const AVDictionaryEntry *track;
#endif // ]
  } tag={
#if 0 // [
    .title=av_dict_get(metadata,"TITLE",NULL,AV_DICT_IGNORE_SUFFIX),
#if defined (BG_TRACK_BASENAME_TRACK) // [
    .track=av_dict_get(metadata,"TRACK",NULL,AV_DICT_IGNORE_SUFFIX),
#endif // ]
#else // ] [
    .title=av_dict_get(metadata,"TITLE",NULL,0),
#if defined (BG_TRACK_BASENAME_TRACK) // [
    .track=av_dict_get(metadata,"TRACK",NULL,0),
#endif // ]
#endif // ]
  };

#if defined (BG_TRACK_ID) // [
  int id=tag.track?atoi(tag.track->value):track->album.id;
#else // ] [
  int id=tag.track?atoi(tag.track->value):-1;
#endif // ]
#if defined (_WIN32) && ! defined (BG_WIN32_CREATE_LOCALE) // [
  const char *locale=NULL;
#endif // ]
  // NOTE: under Linux sizeof(wchar_t) is 4 bytes and under Windows
  //   it is 2 bytes.
#if 0 // [
  wchar_t *op;
#endif // ]
  const ffchar_t *ipath,*sfx,*sp;

  if ((BG_FLAGS_EXT_RENAME&param->flags.extension)&&tag.title) {
#if defined (_WIN32) // [
    if (opath) {
      // swprintf() in any case writes a trailing '\0', hence (1+len).
#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (0<id)
        len=swprintf(*opath,1+track->target.pfx.len,L"%02d_",id);
      else
        len=0;
#else // ] [
      swprintf(*opath,1+track->target.pfx.len,
          track->album.id<100?L"%02lu_":L"%lu_",track->album.id);
#endif // ]
      wcscpy(*opath+track->target.pfx.len,track->target.title);
      PBU_FREE_GUARDED(track->target.title);
      track->target.title=NULL;
    }
    else {
      // claculate the length of the intermediate representation.
      len=bg_title_length(tag.title->value,NULL,0);

      // allocate sufficient memory to hold the intermediate representatiion.
      track->target.title=malloc((len+1)*sizeof *track->target.title);

      if (!track->target.title) {
        _DMESSAGE("allocating intermediate path");
        // set error.
        len=-1;
        goto e_path;
      }

      // transform into the intermediate representation.
      bg_title_length(tag.title->value,track->target.title,len);

#if ! defined (BG_WIN32_CREATE_LOCALE) // [
      // we need to switch back locale.
      setlocale(LC_ALL,locale);
#endif // ]

#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (0<id)
        len=swprintf(NULL,0,L"%02d_",id);
      else
        len=0;
#else // ] [
      len=swprintf(NULL,0,
          track->album.id<100?L"%02lu_":L"%lu_",track->album.id);
#endif // ]
      track->target.pfx.len=len;
      len+=wcslen(track->target.title);
    }
#else // ] [
    if (opath) {
      // snprintf() in any case writes a trailing '\0', hence (1+len).
      // I.e. we assume enough memory is allocated for holding a
      // trailing '\0'.
#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (0<id)
        snprintf(*opath,1+track->target.pfx.len,"%02d_",id);
#else // ] [
      snprintf(*opath,1+track->target.pfx.len,
          track->album.id<100?"%02lu_":"%lu_",track->album.id);
#endif // ]
      // we convert the intermediate representation back into utf-8.
      wcstombs(*opath+track->target.pfx.len,track->target.title,
          len-track->target.pfx.len);
      PBU_FREE_GUARDED(track->target.title);
      track->target.title=NULL;
    }
    else {
      // calculate the length for holding the intermediate representation.
      len=bg_title_length(tag.title->value,NULL,0);

      // allocate sufficient memory  in order to hold the intermediate
      // representation.
      track->target.title=malloc((len+1)*sizeof track->target.title[0]);

      if (!track->target.title) {
        _DMESSAGE("allocating intermediate path");
        // set error.
        len=-1;
        goto e_path;
      }

      bg_title_length(tag.title->value,track->target.title,len);

#if defined (BG_TRACK_BASENAME_TRACK) // [
      if (0<id)
        len=snprintf(NULL,0,"%02d_",id);
      else
        len=0;
#else // ] [
      len=snprintf(NULL,0,track->album.id<100?"%02lu_":"%lu_",track->album.id);
#endif // ]
      track->target.pfx.len=len;
      // add the length for holding the title (transformed to lower case.)
      len+=wcstombs(NULL,track->target.title,0);
    }
#endif // ]
  }
  else {
    if (tree->source.basename) {
      ipath=tree->source.basename;

      if (opath)
        memcpy(*opath,ipath,len*sizeof **opath);
      else {
        sfx=NULL;
        sp=FFSTRSTR(ipath,FFL("."));

        while (sp) {
          sfx=sp;
          sp=FFSTRSTR(++sp,FFL("."));
        }

        len=sfx?(size_t)(sfx-ipath):FFSTRLEN(ipath);
      }
    }
    else
      len=0;
  }
e_path:
  return len;
}
#endif // ]

///////////////////////////////////////////////////////////////////////////////
int bg_track_annotation_create(bg_tree_t *tree)
{
  bg_param_t *param=tree->param;
  bg_track_t *track=&tree->track;
  bg_tree_t *parent=bg_tree_annotation_parent(tree);
  size_t len1,len2;
  // before determining the suffix we need to have the file re-opened.
  const ffchar_t *sfx;
  size_t len3;
  size_t size;
  ffchar_t *tp;

  /////////////////////////////////////////////////////////////////////////////
  if (param->overwrite) {
    // including the path separator!
#if 0 // [
    // pointer arithmetics takes into account the element's size!
    len1/=sizeof tree->source.path[0];
#else // ] [
    len1=tree->source.basename-tree->source.path;
#endif // ]
    len2=bg_track_basename(track,tree,NULL,0);
    // before determining the suffix we need to have the file re-opened.
    sfx=bg_track_out_sfx(track,tree);
    len3=sfx?FFSTRLEN(sfx):0u;
    size=len1+(len2?len2+1u:0u)+(len3?len3+2u:0u);
    tree->target.path=tp=malloc(size*sizeof tp[0]);

    if (!tree->target.path) {
      _DMESSAGE("allocating output path");
      goto e_path;
    }

    // we copy the path separator!
    memcpy(tp,tree->source.path,len1*sizeof tree->source.path[0]);
    tp+=len1;

    tree->target.basename=tp;
    bg_track_basename(track,tree,&tp,len2);
    tp+=len2;
    *tp=0;

    if (sfx) {
      *tp++=FFL('.');
      FFSTRCPY(tp,sfx);
      tp+=len3;
    }

    *tp=0;
  }
  else {
    len1=parent&&parent->target.path?FFSTRLEN(parent->target.path):0u;
    len2=bg_track_basename(track,tree,NULL,0);
    // before determining the suffix we need to have the file re-opened.
    sfx=bg_track_out_sfx(track,tree);
    len3=sfx?FFSTRLEN(sfx):0u;
    size=(len1?len1+1u:0u)+(len2?len2+1u:0u)+(len3?len3+2u:0u);
    tree->target.path=tp=malloc(size*sizeof tp[0]);

    if (!tree->target.path) {
      _DMESSAGE("allocating output path");
      goto e_path;
    }

    if (parent&&parent->target.path) {
      FFSTRCPY(tp,parent->target.path);
      tp+=len1;
      *tp++=FFPATHSEP;
    }

    tree->target.basename=tp;
    bg_track_basename(track,tree,&tp,len2);
    tp+=len2;
    *tp=0;

    if (sfx) {
      *tp++=FFL('.');
      FFSTRCPY(tp,sfx);
      tp+=len3;
    }

    *tp=0;
  }

  if (!param->overwrite&&!ff_fcmp(tree->source.path,tree->target.path)) {
    _DMESSAGE("overwriting not permitted. Use option --overwrite");
    goto e_overwrite_target;
  }

  /////////////////////////////////////////////////////////////////////////////
  len2=FFSTRLEN(param->temp_prefix);
  tree->temp.path=tp=malloc((size+len2)*sizeof tp[0]);

  if (!tree->temp.path) {
    _DMESSAGE("allocating temporary path");
    goto e_temp;
  }

  memcpy(tp,tree->target.path,len1*sizeof tp[0]);
  tp+=len1;

  // Guarding by "len1" proposed by Hadrien Lacour <hadrien.lacour@posteo.net>.
  if (len1&&FFPATHSEP!=tp[-1])
    *tp++=FFPATHSEP;

  tree->temp.basename=tp;
  FFSTRCPY(tp,param->temp_prefix);
  tp+=len2;
  FFSTRCPY(tp,tree->target.basename);

  /////////////////////////////////////////////////////////////////////////////
  if (!ff_fcmp(tree->source.path,tree->temp.path)) {
    _DMESSAGE("attempt to overwrite source file");
    goto e_overwrite_temp;
  }

  if (ff_fexists(tree->temp.path)) {
#if defined (_WIN32) // [
    _DMESSAGEV("file \"%S\" exists; either remove it or by means of"
        " option --temp-prefix define a prefix for temporary files different"
        " from \"%S\"",
        tree->temp.path,param->temp_prefix);
#else // ] [
    _DMESSAGEV("file \"%s\" exists; either remove it or by means of"
        " option --temp-prefix define a prefix for temporary files different"
        " from \"%s\"",
        tree->temp.path,param->temp_prefix);
#endif // ]
    goto e_temp_exists;
  }

#if defined (_WIN32) // [
  // if LANG is set to e.g. "en_US.UTF-8" we assume we're run from
  // e.g. MSYS2 shell undestanding UTF-8 otherwise from MS console using
  // codepage OEM. In the latter case we need an OEM representation of
  // the basename.
  if (tree->param->oem&&tree->target.basename) {
    ///////////////////////////////////////////////////////////////////////////
    track->target.oem.basename=bg_wcs2str(tree->target.basename,CP_OEMCP);

    if (!track->target.oem.basename) {
      _DMESSAGE("creating oem basename");
      goto e_basename;
    }
  }
  else
    track->target.oem.basename=NULL;

  /////////////////////////////////////////////////////////////////////////////
#if defined (BG_WIN32_TARGET_UTF8) // [
  track->target.utf8.path=bg_wcs2str(tree->target.path,CP_UTF8);
  
  if (!track->target.utf8.path) {
    _DMESSAGE("creating utf-8 representation of path");
    goto e_patha;
  }

  track->target.utf8.basename=bg_basenamea(track->target.utf8.path);
#else // ] [
  track->target.path=bg_wcs2str(tree->target.path,CP_UTF8);
  
  if (!track->target.path) {
    _DMESSAGE("creating utf-8 representation of path");
    goto e_patha;
  }

  track->target.basename=bg_basenamea(track->target.path);
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  track->temp.path=bg_wcs2str(tree->temp.path,CP_UTF8);
  
  if (!track->temp.path) {
    _DMESSAGE("creating utf-8 representation of temporary path");
    goto e_tempa;
  }
#endif // ]

  return 0;
//cleanup:
#if defined (_WIN32) // [
  PBU_FREE_GUARDED(track->temp.path);
e_tempa:
#if defined (BG_WIN32_TARGET_UTF8) // [
  PBU_FREE_GUARDED(track->target.utf8.path);
#else // ] [
  PBU_FREE_GUARDED(track->target.path);
#endif // ]
e_patha:
  if (track->target.oem.basename)
    PBU_FREE_GUARDED(track->target.oem.basename);
e_basename:
#endif // ]
e_temp_exists:
e_overwrite_temp:
  PBU_FREE_GUARDED(tree->temp.path);
e_temp:
e_overwrite_target:
  PBU_FREE_GUARDED(tree->target.path);
e_path:
  return -1;
}

FFUNUSED static void bg_PBU_FREE_GUARDED_guarded(void **p)
{
  if (*p) {
    PBU_FREE_GUARDED(*p);
    *p=NULL;
  }
}

void bg_track_annotation_destroy(bg_tree_t *tree FFUNUSED)
{
#if defined (_WIN32) // [
  bg_track_t *track=&tree->track;

  PBU_FREE_GUARDED(track->temp.path);
#if defined (BG_WIN32_TARGET_UTF8) // [
  PBU_FREE_GUARDED(track->target.utf8.path);
#else // ] [
  PBU_FREE_GUARDED(track->target.path);
#endif // ]
  if (track->target.oem.basename)
    PBU_FREE_GUARDED(track->target.oem.basename);
#endif // ]
  PBU_FREE_GUARDED(tree->temp.path);
  PBU_FREE_GUARDED(tree->target.path);
}

///////////////////////////////////////////////////////////////////////////////
int bg_album_annotation_create(bg_tree_t *tree)
{
  bg_tree_t *parent=bg_tree_annotation_parent(tree);
  const ffchar_t *basename=tree->source.basename;
  size_t len1,len2,size;
  ffchar_t *tp;

  /////////////////////////////////////////////////////////////////////////////
  if (!parent)
    goto success;

  if (tree->param->output.dirname) {
    ///////////////////////////////////////////////////////////////////////////
    len1=parent&&parent->target.path?FFSTRLEN(parent->target.path):0u;
    len2=basename?FFSTRLEN(basename):0u;
    size=(len1?len1+1u:0u)+(len2?len2+1u:0u);

    ///////////////////////////////////////////////////////////////////////////
    if (0u<size) {
      /////////////////////////////////////////////////////////////////////////
      tree->target.path=tp=malloc(size*sizeof tp[0]);

      if (!tree->target.path) {
        _DMESSAGE("allocating output path");
        goto e_path;
      }

      if (parent&&parent->target.path) {
        FFSTRCPY(tp,parent->target.path);
        tp+=len1;

        if (basename)
          *tp++=FFPATHSEP;
        else
          *tp=FFL('\0');
      }

      if (basename) {
        tree->target.basename=tp;
        FFSTRCPY(tp,basename);
        tp+=len2;
      }
      else
        tree->target.basename=NULL;

      /////////////////////////////////////////////////////////////////////////
      if (!tree->param->overwrite&&ff_mkdir(tree->target.path)<0) {
        _DMESSAGE("creating directory");
        goto e_mkdir;
      }
    }
    else {
      tree->target.basename=NULL;
      tree->target.path=NULL;
    }
  }
  else {
    tree->target.basename=NULL;
    tree->target.path=NULL;
  }

  /////////////////////////////////////////////////////////////////////////////
success:
  return 0;
//cleanup:
#if 1 // [
e_mkdir:
#endif // ]
  if (tree->target.path)
    PBU_FREE_GUARDED(tree->target.path);
e_path:
  return -1;
}

void bg_album_annotation_destroy(bg_tree_t *tree FFUNUSED)
{
  if (tree->target.path)
    PBU_FREE_GUARDED(tree->target.path);
}


///////////////////////////////////////////////////////////////////////////////
int bg_root_annotation_create(bg_tree_t *tree)
{
  if (tree->param->output.dirname) {
    ///////////////////////////////////////////////////////////////////////////
    tree->target.path=tree->param->output.dirname;

    if (!tree->param->overwrite&&ff_mkdir(tree->target.path)<0) {
      /////////////////////////////////////////////////////////////////////////
      _DMESSAGE("creating directory");
      goto e_mkdir;
    }

    tree->target.basename=NULL;
  }
  else {
    tree->target.path=NULL;
    tree->target.basename=NULL;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
// cleanup:
e_mkdir:
  return -1;
}

void bg_root_annotation_destroy(bg_tree_t *tree FFUNUSED)
{
}
