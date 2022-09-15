/*
 * bgx.c
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
#if defined (_WIN32) // [
#include <fcntl.h>
#include <getoptW.h>
#else // ] [
#include <ctype.h>
#include <getopt.h>
#if defined (__linux__) && defined (__GNUC__) // [
#include <gnu/libc-version.h>
#endif // ]
#endif // ]
#include <locale.h>
#include <bg.h>
#if defined (HAVE_FF_DYNLOAD) // [
#include "bg_version.h"
#endif // ]

///////////////////////////////////////////////////////////////////////////////
/*
 * once meant as an example program on how to deal with "libbg.a" (and in
 * turn with "libff.a") finally serving as bs1770gain's implementation.
 */
#if 1 // [
#if 1 // [
#define BG_GARBAGE \
    ""
#else // ] [
#define BG_GARBAGE \
    ""
#endif // ]
#define BG_NANO_GARBAGE \
""
#endif // ]

///////////////////////////////////////////////////////////////////////////////
static const ffchar_t *bg_version(const ffchar_t *path, FILE *f)
{
  path=FFBASENAME(path);

  _FPRINTFV(f,"%" PBU_PRIs,path);
#if defined (PACKAGE_VERSION) // [
  _FPRINTFV(f," %s",PACKAGE_VERSION);
#endif // ]

#if defined (BG_NANO_GARBAGE) // [
  _FPRINTFV(f,"%s",BG_NANO_GARBAGE);
#endif // ]
#if defined (PACKAGE_URL) // [
  _FPRINTFV(f,"%s\n",PACKAGE_URL);
#endif // ]

#if defined (_WIN32) // [
#if defined (_WIN64) // [
  _FPRINTFV(f,"%s","Compiled for Windows 64 bit");
#else // ] [
  _FPRINTFV(f,"%s","Compiled for Windows 32 bit");
#endif // ]
#elif defined (HAVE_FF_DYNLOAD) && defined (BG_POSIX_SYSNAME) // ] [
  _FPRINTFV(f,"Compiled for %s",BG_POSIX_SYSNAME);
#else // ] [
  _FPRINTFV(f,"%s","Compiled");
#endif // ]
#if defined (__GNUC__) // [
  _FPRINTFV(f," by means of gcc %d.%d.%d",__GNUC__,__GNUC_MINOR__,
      __GNUC_PATCHLEVEL__);
#endif // ]
#if defined (BG_WINDOWS_MAJOR) // [
#if defined (BG_WINDOWS_CSD_VESIONA) // [
  if (BG_WINDOWS_CSD_VESIONA[0]) {
    _FPRINTFV(f," on Windows %d.%d.%d\n(%s) expecting\n",
        BG_WINDOWS_MAJOR,
        BG_WINDOWS_MINOR,
        BG_WINDOWS_BUILD_NUMBER,
        BG_WINDOWS_CSD_VESIONA);
  }
  else {
#endif // ]
    _FPRINTFV(f," on Windows %d.%d.%d\nexpecting\n",
        BG_WINDOWS_MAJOR,
        BG_WINDOWS_MINOR,
        BG_WINDOWS_BUILD_NUMBER);
#if defined (BG_WINDOWS_CSD_VESIONA) // [
  }
#endif // ]
#elif defined (HAVE_FF_DYNLOAD) // ] [
  _FPRINTF(f," on\n");

  if (BG_POSIX_NODENAME[0])
    _FPRINTFV(f,"       nodename:  %s,\n",BG_POSIX_NODENAME);

  if (BG_POSIX_RELEASE[0])
    _FPRINTFV(f,"        release:  %s,\n",BG_POSIX_RELEASE);

  if (BG_POSIX_VERSION[0])
    _FPRINTFV(f,"        version:  %s,\n",BG_POSIX_VERSION);

  if (BG_POSIX_MACHINE[0])
    _FPRINTFV(f,"        machine:  %s,\n",BG_POSIX_MACHINE);

#if defined (BG_POSIX_DOMAINNAME) // [
  if (BG_POSIX_DOMAINNAME[0])
    _FPRINTFV(f,"     domainname:  %s,\n",BG_POSIX_DOMAINNAME);
#endif // ]

  _FPRINTF(f,"expecting\n");
#else // ] [
  _FPRINTF(f," expecting\n");
#endif // ]

#if defined (HAVE_FF_DYNLOAD) && defined (BG_GNU_LIBC_VERSION) // [
  _FPRINTFV(f,"           libc:  %s (%s),\n",
      BG_GNU_LIBC_VERSION,BG_GNU_LIBC_RELEASE);
#endif // ]
  _FPRINTFV(f,"      libavutil:  %d.%d.%d,\n",
      LIBAVUTIL_VERSION_MAJOR,
      LIBAVUTIL_VERSION_MINOR,
      LIBAVUTIL_VERSION_MICRO);
  _FPRINTFV(f,"  libswresample:  %d.%d.%d,\n",
      LIBSWRESAMPLE_VERSION_MAJOR,
      LIBSWRESAMPLE_VERSION_MINOR,
      LIBSWRESAMPLE_VERSION_MICRO);
  _FPRINTFV(f,"     libavcodec:  %d.%d.%d,\n",
      LIBAVCODEC_VERSION_MAJOR,
      LIBAVCODEC_VERSION_MINOR,
      LIBAVCODEC_VERSION_MICRO);
  _FPRINTFV(f,"    libavformat:  %d.%d.%d,\n",
      LIBAVFORMAT_VERSION_MAJOR,
      LIBAVFORMAT_VERSION_MINOR,
      LIBAVFORMAT_VERSION_MICRO);
  _FPRINTFV(f,"     libswscale:  %d.%d.%d,\n",
      LIBSWSCALE_VERSION_MAJOR,
      LIBSWSCALE_VERSION_MINOR,
      LIBSWSCALE_VERSION_MICRO);
  _FPRINTFV(f,"    libpostproc:  %d.%d.%d, and\n",
      LIBPOSTPROC_VERSION_MAJOR,
      LIBPOSTPROC_VERSION_MINOR,
      LIBPOSTPROC_VERSION_MICRO);
  _FPRINTFV(f,"    libavfilter:  %d.%d.%d.\n",
      LIBAVFILTER_VERSION_MAJOR,
      LIBAVFILTER_VERSION_MINOR,
      LIBAVFILTER_VERSION_MICRO);
  _FPRINTF(f,"This is free software; see the source for copying conditions."
      "  There is NO\n"
      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR"
      " PURPOSE.\n");
  return path;
}

static void bg_usage(const ffchar_t *path, FILE *f)
{
  path=bg_version(path,f);

  _FPRINTFV(f,"\nUsage: %" PBU_PRIs,path);

  _FPRINTF(f," [options] <file/dir> [<file/dir> ...]\n\n");
  _FPRINTF(f,"Options:\n");
  _FPRINTF(f," -v,--version:  print this message and exit\n");
  _FPRINTF(f," -h,--help[=<topic>]?:  print this message and exit\n"
      "    (optional argument <topic> out of \"codec\" or \"suffix\")\n");
  /////////////////////////////////////////////////////////////////////////////
  _FPRINTF(f," -i,--integrated:  calculate integrated loudness\n");
  _FPRINTF(f," -s,--shortterm:  calculate maximum shortterm loudness\n");
  _FPRINTF(f," -m,--momentary:  calculate maximum momentary loudness\n");
  _FPRINTF(f," -r,--range:  calculate loudness range\n");
  _FPRINTF(f," -p,--sample-peak:  calculate maximum sample peak\n");
  _FPRINTF(f," -t,--true-peak:  calculate maximum true peak\n");
  _FPRINTF(f," -b <timestamp>,--begin=<timestamp>:  begin decoding at\n"
      "   timestamp (in microseconds, format: hh:mm:ss.ms)\n");
  _FPRINTF(f," -d <duration>,--duration=<duration>:  let decoding\n"
      "   last duration (in microseconds, format: hh:mm:ss.ms)\n");
  _FPRINTF(f," -u <method>,--use=<method>:  base replaygain calculation on\n"
      "   <method> (with respect to the -a/--apply and -o/--output\n"
      "   options. available methods:\n"
      "     integrated (default),\n"
      "     momentary, or\n"
      "     shortterm\n"
      "   experimental methods:\n"
      "     momentary-mean (synonym for \"integrated\"),\n"
      "     momentary-maximum (synonym for \"momentary\"),\n"
      "     shortterm-mean, or\n"
      "     shortterm-maximum (synonym for \"shortterm\")\n");
  _FPRINTF(f," -a [<weight>]/--apply[=<weight>]:  apply the EBU/ATSC/RG\n"
      "   album gain (in conjunction with the -o/--output option.)\n"
      "   when <weight> out of [0.0 .. 1.0] is provided: album gain\n"
      "   plus <weight>*(track gain - album gain).\n");
  _FPRINTF(f," -o <folder>,--output=<folder>:  write replaygain tags\n"
      "   or apply the EBU/ATSC/RG gain, respectively,\n"
      "   and output to <folder>.\n"
      "   specify either option -o/--output or option --overwrite\n"
      "   but not both.\n");
#if defined (_WIN32) // [
  _FPRINTF(f," -f <file>,--file=<file>:  write analysis to an utf-8 log"
      " file\n");
  _FPRINTF(f," --utf-16[=<file>]:  write an utf-16 log file instead of an"
      " utf-8\n"
      "   log file (<file> can be omitted when provided by means of the\n"
      "   -f/--file option.)\n");
#else // ] [
  _FPRINTF(f," -f <file>,--file=<file>:  write analysis to a log file\n");
#endif // ]
#if defined (BG_PARAM_REFERENCE) // [
  _FPRINTF(f," --reference=<path to a refernce tree>:  path to a tree\n"
      "   structural equivalent to the input tree and its's leafes may\n"
      "   serve as a reference to it's correponding input leafes.\n");
#endif // ]
#if defined (BG_PARAM_SKIP_SCAN) // [
  _FPRINTF(f," --skip-scan:  skip counting files.\n");
#endif // ]
#if defined (BG_PARAM_SCRIPT) // [
  _FPRINTF(f," --script=<script>:  run <script> on track. if not explicitely\n"
      "   provided implies option --apply. <script> might be provided\n"
      "   as a string or as a file name.\n");
#endif // ]
#if defined (BG_PARAM_SHELL) // [
  _FPRINTF(f," --shell=<path to interpreter>:  interpret <script> according\n"
      "   to interpreter.\n");
#endif // ]
#if defined (BG_PARAM_QUIET) // [
  _FPRINTF(f," --quiet:  supress any output except error messages.\n");
#endif // ]
#if defined (BG_PARAM_THREADS) // [
  _FPRINTF(f," --threads=<number of threads>:  parallelize using <number\n"
      "     of threads> threads. implies --quiet.\n");
#endif // ]
  /////////////////////////////////////////////////////////////////////////////
  _FPRINTF(f," --ebu:  calculate replay gain according to EBU R128\n"
      "   (-23.0 LUFS, default)\n");
  _FPRINTF(f," --atsc:  calculate replay gain according to ATSC A/85\n"
      "   (-24.0 LUFS)\n");
  _FPRINTF(f," --replaygain:  calculate replay gain according to\n"
      "   ReplayGain 2.0 (-18.0 LUFS)\n");
  _FPRINTF(f," --track-tags:  write track tags\n");
  _FPRINTF(f," --album-tags:  write album tags\n");
  _FPRINTF(f," --tag-prefix=<prefix>:  instead of \"REPLAYGAIN\",\n"
      "   use <prefix> as replaygain tag prefix\n");
  _FPRINTF(f," --unit=<unit>:  write results and tags with <unit> out of\n"
      "   \"ebu\" or \"db\"\n");
  _FPRINTF(f," --audio=<index>/--ai=<index>:  select audio index"
      " (corresponds\n"
      "   to [0:<index>] in FFmpeg listing, cf. -l/--list option)\n");
  _FPRINTF(f," --video=<index>/--vi=<index>:  select video index"
      " (corresponds\n"
      "   to [0:<index>] in FFmpeg listing, cf. -l/--list option)\n");
  _FPRINTF(f," --matrix:<matrix>:  remix to <matrix> out of\n"
      "    front-left,\n"
      "    front-right,\n"
      "    front-center,\n"
      "    low-frequency,\n"
      "    back-left,\n"
      "    back-right,\n"
      "    front-left-of-center,\n"
      "    front-right-of-center,\n"
      "    back-center,\n"
      "    side-left,\n"
      "    side-right,\n"
      "    top-center,\n"
      "    top-front-left,\n"
      "    top-front-center,\n"
      "    top-front-right,\n"
      "    top-back-left,\n"
      "    top-back-center,\n"
      "    top-back-right,\n"
      "    stereo-left,\n"
      "    stereo-right,\n"
      "    wide-left,\n"
      "    wide-right,\n"
      "    surround-direct-left,\n"
      "    surround-direct-right,\n"
      "    low-frequency-2,\n"
      "    layout-mono,\n"
      "    layout-stereo,\n"
      "    layout-2point1,\n"
      "    layout-2-1,\n"
      "    layout-surround,\n"
      "    layout-3point1,\n"
      "    layout-4point0,\n"
      "    layout-4point1,\n"
      "    layout-2-2,\n"
      "    layout-quad,\n"
      "    layout-5point0,\n"
      "    layout-5point1,\n"
      "    layout-5point0-back,\n"
      "    layout-5point1-back,\n"
      "    layout-6point0,\n"
      "    layout-6point0-front,\n"
      "    layout-hexagonal,\n"
      "    layout-6point1,\n"
      "    layout-6point1-back,\n"
      "    layout-6point1-front,\n"
      "    layout-7point0,\n"
      "    layout-7point0-front,\n"
      "    layout-7point1,\n"
      "    layout-7point1-wide,\n"
      "    layout-7point1-wide-back,\n"
      "    layout-octagonal,\n"
      "    layout-hexadecagonal, or\n"
      "    layout-stereo-downmix\n");
  _FPRINTF(f," --stereo:  shorthand for --matrix=layout-stereo\n");
  _FPRINTF(f," --drc=<float>:  set AC3 dynamic range compression (DRC)\n");
  _FPRINTF(f," -x [<extension>]?, --extension[=<extension>]?:  enable"
      " extension\n"
      "   out of\n"
      "    rename:  rename files according to TITLE tag\n"
      "    csv:  read metadata from per-folder CSV file \"folder.csv\"\n"
      "    copy:  copy non-audio files from source to destination\n"
      "      folder\n"
      "    tags:  automatically add the TRACK and DISC tags\n"
      "    all:  all of the above (can be omitted)\n");
  _FPRINTF(f," --suffix=<suffix>:  output to <basename>.<suffix>\n"
      "    (only useful in conjunction with option -o/--output)\n");
  _FPRINTF(f," --loglevel=<level>:  set FFmpeg loglevel to <level> out of\n"
      "    quiet,\n"
      "    panic,\n"
      "    fatal,\n"
      "    error,\n"
      "    warning,\n"
      "    info,\n"
      "    verbose,\n"
      "    debug, or\n"
      "    trace\n");
  _FPRINTF(f," --xml:  print results in xml format\n");
#if defined (BG_CLOCK) // [
  _FPRINTF(f," --time:  print out duration of program invocation\n");
#endif // ]
  _FPRINTF(f," --norm=<float>:  norm loudness to float.\n");
#if 0 // [
  _FPRINTF(f," --preamp=<preamp>:\n");
  _FPRINTF(f," --stero:\n");
  _FPRINTF(f," --rg-tags:\n");
  _FPRINTF(f," --bwf-tags:\n");
#endif // ]
  _FPRINTF(f," --overwrite:  replace any source file by its respective\n"
      "    target file. specify either option -o/--output or option\n"
      "    --overwrite but not both.\n"
      "    WARNING:  use this option with extrem care. The source\n"
      "      files will definitely be lost!!! For this reason using\n"
      "      this option is discouraged. It's provided just for\n"
      "      completeness.\n");
#if defined (BG_PARAM_LFE) && defined (BG_CHANNEL_LFE) // [
  _FPRINTFV(f," --lfe=<lfe channel>  overwrite the default <lfe"
      " channel>\n"
      "    (default: %d.)\n",BG_CHANNEL_LFE);
#endif // ]
  _FPRINTF(f," --codec=<name>:  use audio codec \"<name>\" for output\n"
      "    default: \"flac\".)\n");
  _FPRINTF(f," --temp-prefix=<prefix>:  create temporary files with\n"
      "    prefix \"<prefix>\" default: \"" BG_TEMP_PREFIX "\".)\n");
  _FPRINTF(f," --suppress-hierarchy:  suppress printing results up the\n"
      "    hierarchy.\n");
  _FPRINTF(f," --suppress-progress:  suppress printing processing"
      " progress\n");
  /////////////////////////////////////////////////////////////////////////////
  _FPRINTF(f,"\nExperimental options:\n");
  ////////
  _FPRINTF(f,"1) momentary block\n");
  _FPRINTF(f," --momentary-mean:  calculate mean loudness based on\n"
      "   momentary block (same as --integrated)\n");
  _FPRINTF(f," --momentary-maximum:  calculate maximum loudness based\n"
      "   on momentary block (same as --momentary)\n");
  _FPRINTF(f," --momentary-range:  calculate loudness range based on\n"
      "   momentary block\n");
  _FPRINTF(f," --momentary-length=<ms>:  length of momentary block\n"
      "   in milliseconds (default: 400)\n");
  _FPRINTF(f," --momentary-overlap=<percent>:  overlap of momentary\n"
      "   block in percent (default: 75)\n");
  _FPRINTF(f," --momentary-mean-gate=<gate>:  silence gate for mean\n"
      "   measurement of momentary block (default: -10.0)\n");
  _FPRINTF(f," --momentary-range-gate=<gate>:  silence gate for range\n"
      "   measurement of momentary block (default: -20.0)\n");
  _FPRINTF(f," --momentary-range-lower-bound=<float>:  lower bound for\n"
      "   range measurement of momentary block (default: 0.1)\n");
  _FPRINTF(f," --momentary-range-upper-bound=<float>:  upper bound for\n"
      "   range measurement of momentary block (default: 0.95)\n");
  ////////
  _FPRINTF(f,"2) shortterm block\n");
  _FPRINTF(f," --shortterm-mean:  calculate mean loudness based on\n"
      "   shortterm block\n");
  _FPRINTF(f," --shortterm-maximum:  calculate maximum loudness based\n"
      "   on shortterm block (same as --shortterm)\n");
  _FPRINTF(f," --shortterm-range:  calculate loudness range based on\n"
      "   shortterm block (same as --range)\n");
  _FPRINTF(f," --shortterm-length=<ms>:  length of shortterm block\n"
      "   in milliseconds (default: 3000)\n");
  _FPRINTF(f," --shortterm-overlap <percent>:  overlap of shortterm\n"
      "   block in percent (default: 67)\n");
  _FPRINTF(f," --shortterm-mean-gate=<gate>:  silence gate for mean\n"
      "   measurement of shortterm block (default: -10.0)\n");
  _FPRINTF(f," --shortterm-range-gate=<gate>:  silence gate for range\n"
      "   measurement of shortterm block (default: -20.0)\n");
  _FPRINTF(f," --shortterm-range-lower-bound=<float>:  lower bound for\n"
      "   range measurement of shortterm block (default: 0.1)\n");
  _FPRINTF(f," --shortterm-range-upper-bound=<float>:  upper bound for\n"
      "   range measurement of shortterm block (default: 0.95)\n");
  /////////////////////////////////////////////////////////////////////////////
  _FPRINTF(f,"\n");
  _FPRINTF(f,"Command line arguments may appear in any order.\n");
}

#if defined (HAVE_FF_DYNLOAD) && defined (__linux__) \
    && defined (__GNUC__) // [
///////////////////////////////////////////////////////////////////////////////
static int strcmpex(const char **s1, const char **s2, int ch)
{
  for (;;) {
    int lhs=ch==**s1?0:**s1;
    int rhs=ch==**s2?0:**s2;
    int cmp=lhs-rhs;

    if (cmp||!lhs||!rhs) {
      if (ch==**s1)
        ++*s1;

      if (ch==**s2)
        ++*s2;

      return cmp;
    }
    else {
      ++*s1;
      ++*s2;
    }
  }
}
#endif // ]

///////////////////////////////////////////////////////////////////////////////
// https://stackoverflow.com/questions/190543/how-can-i-change-the-width-of-a-windows-console-window
// https://docs.microsoft.com/en-us/windows/console/window-and-screen-buffer-size
// David Tran: Logging Function Entry and Exit in C
//  https://davidtranscend.com/blog/log-function-entry-exit-c/
#if defined (_WIN32) // [
int wmain(int argc, wchar_t *const *argv)
#else // ] [
int main(int argc, char *const *argv)
#endif // ]
{
  enum {
#if 0 // [
    BG_ARG_APPLY=FFL('z')+1,
    BG_ARG_UNIT,
#else // ] [
    BG_ARG_UNIT=FFL('z')+1,
#endif // ]
#if defined (_WIN32) // [
    BG_ARG_UTF_16,
#endif // ]
#if defined (BG_PARAM_REFERENCE) // [
    BG_REFERENCE,
#endif // ]
#if defined (BG_PARAM_SKIP_SCAN) // [
    BG_SKIP_SCAN,
#endif // ]
#if defined (BG_PARAM_SCRIPT) // [
    BG_SCRIPT,
#endif // ]
#if defined (BG_PARAM_SHELL) // [
    BG_SHELL,
#endif // ]
    BG_ARG_DRC,
    BG_ARG_LOGLEVEL,
    BG_ARG_NORM,
    BG_ARG_PREAMP,
#if defined (BG_CLOCK) // [
    BG_ARG_TIME,
#endif // ]
    BG_ARG_EBU,
    BG_ARG_SUFFIX,
    BG_ARG_ATSC,
    BG_ARG_AUDIO,
    BG_ARG_VIDEO,
    BG_ARG_MATRIX,
    BG_ARG_STEREO,
    BG_ARG_REPLAYGAIN,
    BG_ARG_RG_TAGS,
#if defined (BG_BWF_TAGS) // [
    BG_ARG_BWF_TAGS,
#endif // ]
    BG_ARG_TAGS_TRACK,
    BG_ARG_TAGS_ALBUM,
    BG_ARG_XML,
    BG_ARG_TAG_PREFIX,
    BG_ARG_OVERWRITE,
#if defined (BG_PARAM_LFE) // [
    BG_ARG_CH_LFE,
#endif // ]
    BG_ARG_OUTPUT_CODEC,
    BG_ARG_TEMP_PREFIX,
    BG_ARG_SUPPRESS_HIERARCHY,
    BG_ARG_SUPPRESS_PROGRESS,
#if defined (BG_PARAM_QUIET) // [
    BG_ARG_QUIET,
#endif // ]
#if defined (BG_PARAM_THREADS) // [
    BG_ARG_NTHREADS,
#endif // ]
#if defined (BG_PARAM_SLEEP) // [
    BG_ARG_SLEEP,
#endif // ]
    BG_ARG_EXTENSION,
    ////////////////
#if 0 // [
    BG_ARG_MOMENTARY_MEAN,
    BG_ARG_MOMENTARY_MAXIMUM,
#endif // ]
    BG_ARG_MOMENTARY_RANGE,
    BG_ARG_MOMENTARY_LENGTH,
    BG_ARG_MOMENTARY_OVERLAP,
    BG_ARG_MOMENTARY_MEAN_GATE,
    BG_ARG_MOMENTARY_RANGE_GATE,
    BG_ARG_MOMENTARY_RANGE_LOWER_BOUND,
    BG_ARG_MOMENTARY_RANGE_UPPER_BOUND,
    ////////////////
    BG_ARG_SHORTTERM_MEAN,
#if 0 // [
    BG_ARG_SHORTTERM_MAXIMUM,
    BG_ARG_SHORTTERM_RANGE,
#endif // ]
    BG_ARG_SHORTTERM_LENGTH,
    BG_ARG_SHORTTERM_OVERLAP,
    BG_ARG_SHORTTERM_MEAN_GATE,
    BG_ARG_SHORTTERM_RANGE_GATE,
    BG_ARG_SHORTTERM_RANGE_LOWER_BOUND,
    BG_ARG_SHORTTERM_RANGE_UPPER_BOUND,
  };

  enum BGFlagHelpArg {
    BG_FLAG_HELP_ARG_NULL=1<<0,
    BG_FLAG_HELP_ARG_VERSION=1<<1,
    BG_FLAG_HELP_ARG_SUFFIX=1<<2,
    BG_FLAG_HELP_ARG_CODEC=1<<3,
  };

#if defined (BG_CLOCK) // [
  enum {
    BG_MIN=60,
    BG_HOUR=60*BG_MIN,
  };
#endif // ]

#if defined (HAVE_FF_DYNLOAD) // [
  static const ffchar_t TOOLS[]=FFL("bs1770gain-tools");
#endif // ]
  static const ffchar_t FLAGS[]=
      FFL("hvu:ptismrb:d:o:f:alx");

  static struct option OPTS[]={
    { FFL("version"),no_argument,NULL,FFL('v') },
    { FFL("help"),optional_argument,NULL,FFL('h') },
    { FFL("begin"),required_argument,NULL,FFL('b') },
    { FFL("duration"),required_argument,NULL,FFL('d') },
    { FFL("file"),required_argument,NULL,FFL('f') },
    { FFL("utf-8"),required_argument,NULL,FFL('f') },
    { FFL("output"),required_argument,NULL,FFL('o') },
    { FFL("use"),required_argument,NULL,FFL('u') },
    { FFL("integrated"),no_argument,NULL,FFL('i') },
    { FFL("momentary"),no_argument,NULL,FFL('m') },
    { FFL("range"),no_argument,NULL,FFL('r') },
    { FFL("shortterm"),no_argument,NULL,FFL('s') },
    { FFL("sample-peak"),no_argument,NULL,FFL('p') },
    { FFL("samplepeak"),no_argument,NULL,FFL('p') },
    { FFL("true-peak"),no_argument,NULL,FFL('t') },
    { FFL("truepeak"),no_argument,NULL,FFL('t') },
    { FFL("apply"),optional_argument,NULL,FFL('a') },
#if defined (_WIN32) // [
    { FFL("utf-16"),optional_argument,NULL,BG_ARG_UTF_16 },
#endif // ]
#if defined (BG_PARAM_SKIP_SCAN) // [
    { FFL("skip-scan"),optional_argument,NULL,BG_SKIP_SCAN },
#endif // ]
#if defined (BG_PARAM_REFERENCE) // [
    { FFL("reference"),required_argument,NULL,BG_REFERENCE },
#endif // ]
#if defined (BG_PARAM_SCRIPT) // [
    { FFL("script"),required_argument,NULL,BG_SCRIPT },
#endif // ]
#if defined (BG_PARAM_SHELL) // [
    { FFL("shell"),required_argument,NULL,BG_SHELL },
#endif // ]
    { FFL("unit"),required_argument,NULL,BG_ARG_UNIT },
    { FFL("audio"),required_argument,NULL,BG_ARG_AUDIO },
    { FFL("ai"),required_argument,NULL,BG_ARG_AUDIO },
    { FFL("drc"),required_argument,NULL,BG_ARG_DRC },
#if defined (BG_PARAM_QUIET) // [
    { FFL("quiet"),no_argument,NULL,BG_ARG_QUIET },
#endif // ]
#if defined (BG_PARAM_THREADS) // [
    { FFL("threads"),required_argument,NULL,BG_ARG_NTHREADS },
#endif // ]
#if defined (BG_PARAM_SLEEP) // [
    { FFL("sleep"),required_argument,NULL,BG_ARG_SLEEP },
#endif // ]
    { FFL("extension"),optional_argument,NULL,BG_ARG_EXTENSION },
    { FFL("suffix"),required_argument,NULL,BG_ARG_SUFFIX },
    { FFL("loglevel"),required_argument,NULL,BG_ARG_LOGLEVEL },
    { FFL("norm"),required_argument,NULL,BG_ARG_NORM },
    { FFL("preamp"),required_argument,NULL,BG_ARG_PREAMP },
    { FFL("video"),required_argument,NULL,BG_ARG_VIDEO },
    { FFL("vi"),required_argument,NULL,BG_ARG_VIDEO },
    { FFL("time"),no_argument,NULL,BG_ARG_TIME },
    { FFL("ebu"),no_argument,NULL,BG_ARG_EBU },
    { FFL("atsc"),no_argument,NULL,BG_ARG_ATSC },
    { FFL("matrix"),required_argument,NULL,BG_ARG_MATRIX },
    { FFL("stereo"),no_argument,NULL,BG_ARG_STEREO },
    { FFL("replaygain"),no_argument,NULL,BG_ARG_REPLAYGAIN },
#if defined (BG_BWF_TAGS) // [
    { FFL("rg-tags"),no_argument,NULL,BG_ARG_TAGS_RG },
    { FFL("bwf-tags"),no_argument,NULL,BG_ARG_TAGS_BWF },
#endif // ]
    { FFL("track-tags"),no_argument,NULL,BG_ARG_TAGS_TRACK },
    { FFL("album-tags"),no_argument,NULL,BG_ARG_TAGS_ALBUM },
    { FFL("xml"),no_argument,NULL,BG_ARG_XML },
    { FFL("tag-prefix"),required_argument,NULL,BG_ARG_TAG_PREFIX },
    { FFL("overwrite"),no_argument,NULL,BG_ARG_OVERWRITE },
#if defined (BG_PARAM_LFE) // [
    { FFL("lfe"),required_argument,NULL,BG_ARG_CH_LFE },
#endif // ]
    { FFL("codec"),required_argument,NULL,BG_ARG_OUTPUT_CODEC },
    { FFL("temp-prefix"),required_argument,NULL,BG_ARG_TEMP_PREFIX },
    { FFL("suppress-hierarchy"),no_argument,NULL,BG_ARG_SUPPRESS_HIERARCHY },
    { FFL("suppress-progress"),no_argument,NULL,BG_ARG_SUPPRESS_PROGRESS },
    // momentary //////////////////////////////////////////////////////////////
    { FFL("momentary-mean"),no_argument,NULL,
        FFL('i') },
    { FFL("momentary-maximum"),no_argument,NULL,
        FFL('m') },
    { FFL("momentary-range"),no_argument,NULL,
        BG_ARG_MOMENTARY_RANGE },
    { FFL("momentary-length"),required_argument,NULL,
        BG_ARG_MOMENTARY_LENGTH },
    { FFL("momentary-overlap"),required_argument,NULL,
        BG_ARG_MOMENTARY_OVERLAP },
    { FFL("momentary-mean-gate"),required_argument,NULL,
        BG_ARG_MOMENTARY_MEAN_GATE },
    { FFL("momentary-range-gate"),required_argument,NULL,
        BG_ARG_MOMENTARY_RANGE_GATE },
    { FFL("momentary-range-lower-bound"),required_argument,NULL,
        BG_ARG_MOMENTARY_RANGE_LOWER_BOUND },
    { FFL("momentary-range-upper-bound"),required_argument,NULL,
        BG_ARG_MOMENTARY_RANGE_UPPER_BOUND },
    // shortterm //////////////////////////////////////////////////////////////
    { FFL("shortterm-mean"),no_argument,NULL,
        BG_ARG_SHORTTERM_MEAN },
    { FFL("shortterm-maximum"),no_argument,NULL,
        FFL('s') },
    { FFL("shortterm-range"),no_argument,NULL,
        FFL('r') },
    { FFL("shortterm-length"),required_argument,NULL,
        BG_ARG_SHORTTERM_LENGTH },
    { FFL("shortterm-overlap"),required_argument,NULL,
        BG_ARG_SHORTTERM_OVERLAP },
    { FFL("shortterm-mean-gate"),required_argument,NULL,
        BG_ARG_SHORTTERM_MEAN_GATE },
    { FFL("shortterm-range-gate"),required_argument,NULL,
        BG_ARG_SHORTTERM_RANGE_GATE },
    { FFL("shortterm-range-lower-bound"),required_argument,NULL,
        BG_ARG_SHORTTERM_RANGE_LOWER_BOUND },
    { FFL("shortterm-range-upper-bound"),required_argument,NULL,
        BG_ARG_SHORTTERM_RANGE_UPPER_BOUND },
    { 0 }
  };

  int code=1;
#if defined (_WIN32) // [
  int utf16=0;
  // if LANG is set to e.g. "en_US.UTF-8" we assume we're run from
  // e.g. MSYS2 shell undestanding UTF-8 otherwise from MS console using
  // codepage OEM.
  const char *lang=getenv("LANG");
#else // ] [
  const char *locale=NULL;
#endif // ]
  const ffchar_t *fpath=NULL;
  enum BGFlagHelpArg help_args=0;
#if defined (BG_PARAM_SCRIPT) // [
  ffchar_t *script=NULL;
#endif // ]
#if defined (HAVE_FF_DYNLOAD) && defined (__linux__) && defined (__GNUC__) // [
  const char *lhs;
  const char *rhs;
#endif // ]
  bg_param_t param;
  int c;
  double overlap;
  unsigned version;
#if (58<=LIBAVCODEC_VERSION_MAJOR||58<=LIBAVFORMAT_VERSION_MAJOR) // [
  void *opaque;
#endif // ]
  const AVCodec *codec;
  const AVOutputFormat *oformat;
#if defined (BG_CLOCK) // [
  double t1,t2,sec;
#endif // ]
  int verbose;

#if defined (PBU_CONSOLE_UTF8) && defined (_WIN32) // [
  /////////////////////////////////////////////////////////////////////////////
#if 0 // [
  _setmode(_fileno(stdout), _O_U16TEXT);
  _setmode(_fileno(stderr), _O_U16TEXT);
#else // ] [
  SetConsoleOutputCP(CP_UTF8);
#endif // ]
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (argc<2) {
    _DMESSAGE("missing command line arguments.");
    _FPRINTF(stderr,"\n");
    bg_usage(argv[0],stderr);
    goto e_arg;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_param_create(&param)<0) {
    _DMESSAGE("create parameters");
    goto e_arg;
  }

  /////////////////////////////////////////////////////////////////////////////
  while (0<=(c=FF_GETOPT_LONG(argc,argv,FLAGS,OPTS,NULL))) {
    switch (c) {
    ///////////////////////////////////////////////////////////////////////////
    case FFL('?'):
      _FPRINTF(stderr,"\n\n");
      bg_usage(argv[0],stderr);
      goto e_arg;
    ///////////////////////////////////////////////////////////////////////////
    case FFL('v'):
#if 0 // [
      bg_version(argv[0],stdout);
      code=0;
      goto e_arg;
#else // ] [
      help_args|=BG_FLAG_HELP_ARG_VERSION;
      break;
#endif // ]
    case FFL('h'):
      if (!optarg)
        help_args|=BG_FLAG_HELP_ARG_NULL;
      else if (!FFSTRCMP(FFL("codec"),optarg))
        help_args|=BG_FLAG_HELP_ARG_CODEC;
      else if (!FFSTRCMP(FFL("suffix"),optarg))
        help_args|=BG_FLAG_HELP_ARG_SUFFIX;
      else {
        _FPRINTFV(stderr,"Error: help topic \"%" PBU_PRIs "\" not"
            " recognized.\n\n",optarg);
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      break;
    case FFL('b'):
      param.interval.begin=bg_parse_time(optarg);
      break;
    case FFL('d'):
      param.interval.duration=bg_parse_time(optarg);
      break;
    case FFL('f'):
      fpath=optarg;
      break;
    case FFL('o'):
      if (!optarg) {
        _DMESSAGE("missing argument to option -o/--output");
        goto e_arg;
      }
      else if (param.overwrite) {
        _DMESSAGE("specify either option -o/--output or option --overwrite"
            " but not both");
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      param.output.dirname=bg_pathnorm(optarg);
      break;
    case FFL('u'):
      if (!FFSTRCASECMP(FFL("integrated"),optarg)
          ||!FFSTRCASECMP(FFL("momentary-mean"),optarg))
        param.flags.aggregate|=BG_FLAGS_AGG_MOMENTARY_MEAN;
      else if (!FFSTRCASECMP(FFL("momentary"),optarg)
          ||!FFSTRCASECMP(FFL("momentary-maximum"),optarg))
        param.flags.aggregate|=BG_FLAGS_AGG_MOMENTARY_MAXIMUM;
      else if (!FFSTRCASECMP(FFL("shortterm-mean"),optarg))
        param.flags.aggregate|=BG_FLAGS_AGG_SHORTTERM_MEAN;
      else if (!FFSTRCASECMP(FFL("shortterm"),optarg)
          ||!FFSTRCASECMP(FFL("shortterm-maximum"),optarg))
        param.flags.aggregate|=BG_FLAGS_AGG_SHORTTERM_MAXIMUM;
      else {
        _DMESSAGEV("method \"%" PBU_PRIs "\" not recognized",optarg);
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      break;
    /// without argument ////////////////////////////////////////////////////
    case FFL('a'):
      param.flags.mode|=BG_FLAGS_MODE_APPLY;

      if (optarg) {
        param.weight.enabled=1;
        param.weight.value=FFATOF(optarg);

        if (param.weight.value<0.0||1.0<param.weight.value) {
          _DMESSAGE("weight out of range");
          bg_usage(argv[0],stderr);
          goto e_arg;
        }
      }

      break;
    case FFL('i'):
      param.flags.aggregate|=BG_FLAGS_AGG_MOMENTARY_MEAN;
      break;
    case FFL('s'):
      param.flags.aggregate|=BG_FLAGS_AGG_SHORTTERM_MAXIMUM;
      break;
    case FFL('m'):
      param.flags.aggregate|=BG_FLAGS_AGG_MOMENTARY_MAXIMUM;
      break;
    case FFL('r'):
      param.flags.aggregate|=BG_FLAGS_AGG_SHORTTERM_RANGE;
      break;
    case FFL('p'):
      param.flags.aggregate|=BG_FLAGS_AGG_SAMPLEPEAK;
      break;
    case FFL('t'):
      param.flags.aggregate|=BG_FLAGS_AGG_TRUEPEAK;
      break;
#if defined (BG_PARAM_QUIET) // [
    case BG_ARG_QUIET:
      param.quiet=1;
      break;
#endif // ]
#if defined (BG_PARAM_THREADS) // [
    case BG_ARG_NTHREADS:
      param.nthreads=FFATOI(optarg);
      break;
#endif // ]
#if defined (BG_PARAM_SLEEP) // [
    case BG_ARG_SLEEP:
      param.sleep=FFATOI(optarg);
      break;
#endif // ]
    case FFL('x'):
    case BG_ARG_EXTENSION:
      if (!optarg) {
        param.flags.aggregate|=BG_FLAGS_AGG_TRUEPEAK;
        param.flags.extension=BG_FLAGS_EXT_ALL;
#if defined (BG_PARAM_STEREO) // [
        param.stereo=1;
#else // ] [
        param.decode.request.channel_layout=AV_CH_LAYOUT_STEREO;
#endif // ]
        param.decode.drc.enabled=1;
#if 0 // [
        param.decode.drc.scale=0.0;
#endif // ]
      }
      else if (0==FFSTRCASECMP(FFL("2"),optarg)) {
        param.flags.aggregate|=BG_FLAGS_AGG_TRUEPEAK;
        param.flags.extension=BG_FLAGS_EXT_ALL&~BG_FLAGS_EXT_RENAME;
#if defined (BG_PARAM_STEREO) // [
        param.stereo=1;
#else // ] [
        param.decode.request.channel_layout=AV_CH_LAYOUT_STEREO;
#endif // ]
        param.decode.drc.enabled=1;
#if 0 // [
        param.decode.drc.scale=0.0;
#endif // ]
      }
      else if (0==FFSTRCASECMP(FFL("rename"),optarg))
        param.flags.extension|=BG_FLAGS_EXT_RENAME;
      else if (0==FFSTRCASECMP(FFL("csv"),optarg))
        param.flags.extension|=BG_FLAGS_EXT_CSV;
      else if (0==FFSTRCASECMP(FFL("copy"),optarg))
        param.flags.extension|=BG_FLAGS_EXT_COPY;
      else if (0==FFSTRCASECMP(FFL("tags"),optarg))
        param.flags.extension|=BG_FLAGS_EXT_TAGS;
      else if (0==FFSTRCASECMP(FFL("all"),optarg))
        param.flags.extension|=BG_FLAGS_EXT_ALL;
      else {
        _DMESSAGEV("extension \"%" PBU_PRIs "\" not recognized",optarg);
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      break;
    /// without flag ////////////////////////////////////////////////////////
    case BG_ARG_AUDIO:
      param.ai=FFATOI(optarg);
      break;
    case BG_ARG_VIDEO:
      param.vi=FFATOI(optarg);
      break;
    case BG_ARG_UNIT:
      if (!FFSTRCMP(optarg,FFL("ebu")))
        bg_param_set_unit_ebu(&param);
      else if (!FFSTRCMP(optarg,FFL("db")))
        bg_param_set_unit_db(&param);
      else {
        _DMESSAGEV("argument to option --unit not recognized:"
            " \"%" PBU_PRIs "\"",optarg);
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      break;
#if defined (_WIN32) // [
    case BG_ARG_UTF_16:
      utf16=1;

      if (optarg)
        fpath=optarg;

      break;
#endif // ]
#if defined (BG_PARAM_REFERENCE) // [
    case BG_REFERENCE:
      param.reference=optarg;
      break;
#endif // ]
#if defined (BG_PARAM_SKIP_SCAN) // [
    case BG_SKIP_SCAN:
      param.skip_scan=1;
      break;
#endif // ]
#if defined (BG_PARAM_SCRIPT) // [
    case BG_SCRIPT:
      script=optarg;
      break;
#endif // ]
#if defined (BG_PARAM_SHELL) // [
    case BG_SHELL:
      param.shell.interpreter=optarg;
      break;
#endif // ]
    case BG_ARG_DRC:
      param.decode.drc.enabled=1;
      param.decode.drc.scale=FFATOF(optarg);
      break;
    case BG_ARG_SUFFIX:
      param.out.sfx=optarg;
      break;
    case BG_ARG_TAG_PREFIX:
#if defined (_WIN32) // [
      ff_wcs2str(optarg,param.tag.pfx,CP_UTF8,(sizeof param.tag.pfx)-2);
#else // ] [
      strncpy(param.tag.pfx,optarg,(sizeof param.tag.pfx)-2);
#endif // ]
      param.tag.pfx[strlen(param.tag.pfx)+1]='\0';
      param.tag.pfx[strlen(param.tag.pfx)]='_';
      break;
    case BG_ARG_OVERWRITE:
      if (param.output.dirname) {
        _DMESSAGE("specify either option -o/--output or option --overwrite"
            " but not both");
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      param.overwrite=1;
      break;
#if defined (BG_PARAM_LFE) // [
    case BG_ARG_CH_LFE:
      param.lfe=FFATOI(optarg);
      break;
#endif // ]
    case BG_ARG_OUTPUT_CODEC:
#if defined (_WIN32) // [
      ff_wcs2str(optarg,param.codec.name,CP_UTF8,(sizeof param.codec.name)-1);
#else // ] [
      param.codec.name=optarg;
#endif // ]
      break;
    case BG_ARG_TEMP_PREFIX:
      if (!*optarg) {
        _DMESSAGE("missing argument to --temp-prefix");
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      param.temp_prefix=optarg;
      break;
    case BG_ARG_SUPPRESS_HIERARCHY:
      param.suppress.hierarchy=1;
      break;
    case BG_ARG_SUPPRESS_PROGRESS:
      param.suppress.progress=1;
      break;
    case BG_ARG_LOGLEVEL:
      if (0==FFSTRCASECMP(FFL("quiet"),optarg))
        param.loglevel=AV_LOG_QUIET;
      else if (0==FFSTRCASECMP(FFL("panic"),optarg))
        param.loglevel=AV_LOG_PANIC;
      else if (0==FFSTRCASECMP(FFL("fatal"),optarg))
        param.loglevel=AV_LOG_FATAL;
      else if (0==FFSTRCASECMP(FFL("error"),optarg))
        param.loglevel=AV_LOG_ERROR;
      else if (0==FFSTRCASECMP(FFL("warning"),optarg))
        param.loglevel=AV_LOG_WARNING;
      else if (0==FFSTRCASECMP(FFL("info"),optarg))
        param.loglevel=AV_LOG_INFO;
      else if (0==FFSTRCASECMP(FFL("verbose"),optarg))
        param.loglevel=AV_LOG_VERBOSE;
      else if (0==FFSTRCASECMP(FFL("debug"),optarg))
        param.loglevel=AV_LOG_DEBUG;
      else if (0==FFSTRCASECMP(FFL("trace"),optarg))
        param.loglevel=AV_LOG_TRACE;
      else {
        _FPRINTFV(stderr,"Error: loglevel \"%" PBU_PRIs "\""
            " not recognized.\n\n",optarg);
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      break;
    case BG_ARG_NORM:
      param.flags.norm=BG_FLAGS_NORM_NULL;
      param.norm=FFATOF(optarg);
      break;
#if defined (BG_CLOCK) // [
    case BG_ARG_TIME:
      param.time=1;
      break;
#endif // ]
    case BG_ARG_PREAMP:
      param.preamp=FFATOF(optarg);
      break;
    /// without flag ////////////////////////////////////////////////////////
    case BG_ARG_ATSC:
      param.flags.norm=BG_FLAGS_NORM_ATSC;
      param.norm=-24.0;
      bg_param_set_unit_db(&param);
      break;
    case BG_ARG_EBU:
      param.flags.norm=BG_FLAGS_NORM_EBU;
      param.norm=-23.0;
      bg_param_set_unit_ebu(&param);
      break;
    case BG_ARG_REPLAYGAIN:
      param.flags.norm=BG_FLAGS_NORM_REPLAYGAIN;
      param.norm=-18.0;
      bg_param_set_unit_db(&param);
      break;
#if defined (BG_BWF_TAGS) // [
    case BG_ARG_TAGS_RG:
      param.flags.mode|=BG_FLAGS_MODE_TAGS_RG;
      break;
    case BG_ARG_TAGS_BWF:
      param.flags.mode|=BG_FLAGS_MODE_TAGS_BWF;
      break;
#endif // ]
    case BG_ARG_TAGS_TRACK:
      param.flags.mode|=BG_FLAGS_MODE_TAGS_TRACK;
      break;
    case BG_ARG_TAGS_ALBUM:
      param.flags.mode|=BG_FLAGS_MODE_TAGS_ALBUM;
      break;
    case BG_ARG_XML:
      param.print.vmt=&bg_print_xml_vmt;
      break;
    case BG_ARG_MATRIX:
      if (!FFSTRCASECMP(FFL("front-left"),optarg))
        param.decode.request.channel_layout=AV_CH_FRONT_LEFT;
      else if (!FFSTRCASECMP(FFL("front-right"),optarg))
        param.decode.request.channel_layout=AV_CH_FRONT_RIGHT;
      else if (!FFSTRCASECMP(FFL("front-center"),optarg))
        param.decode.request.channel_layout=AV_CH_FRONT_CENTER;
      else if (!FFSTRCASECMP(FFL("low-frequency"),optarg))
        param.decode.request.channel_layout=AV_CH_LOW_FREQUENCY;
      else if (!FFSTRCASECMP(FFL("back-left"),optarg))
        param.decode.request.channel_layout=AV_CH_BACK_LEFT;
      else if (!FFSTRCASECMP(FFL("back-right"),optarg))
        param.decode.request.channel_layout=AV_CH_BACK_RIGHT;
      else if (!FFSTRCASECMP(FFL("front-left-of-center"),optarg))
        param.decode.request.channel_layout=AV_CH_FRONT_LEFT_OF_CENTER;
      else if (!FFSTRCASECMP(FFL("front-right-of-center"),optarg))
        param.decode.request.channel_layout=AV_CH_FRONT_RIGHT_OF_CENTER;
      else if (!FFSTRCASECMP(FFL("back-center"),optarg))
        param.decode.request.channel_layout=AV_CH_BACK_CENTER;
      else if (!FFSTRCASECMP(FFL("side-left"),optarg))
        param.decode.request.channel_layout=AV_CH_SIDE_LEFT;
      else if (!FFSTRCASECMP(FFL("side-right"),optarg))
        param.decode.request.channel_layout=AV_CH_SIDE_RIGHT;
      else if (!FFSTRCASECMP(FFL("top-center"),optarg))
        param.decode.request.channel_layout=AV_CH_TOP_CENTER;
      else if (!FFSTRCASECMP(FFL("top-front-left"),optarg))
        param.decode.request.channel_layout=AV_CH_TOP_FRONT_LEFT;
      else if (!FFSTRCASECMP(FFL("top-front-center"),optarg))
        param.decode.request.channel_layout=AV_CH_TOP_FRONT_CENTER;
      else if (!FFSTRCASECMP(FFL("top-front-right"),optarg))
        param.decode.request.channel_layout=AV_CH_TOP_FRONT_RIGHT;
      else if (!FFSTRCASECMP(FFL("top-back-left"),optarg))
        param.decode.request.channel_layout=AV_CH_TOP_BACK_LEFT;
      else if (!FFSTRCASECMP(FFL("top-back-center"),optarg))
        param.decode.request.channel_layout=AV_CH_TOP_BACK_CENTER;
      else if (!FFSTRCASECMP(FFL("top-back-right"),optarg))
        param.decode.request.channel_layout=AV_CH_TOP_BACK_RIGHT;
      else if (!FFSTRCASECMP(FFL("stereo-left"),optarg))
        param.decode.request.channel_layout=AV_CH_STEREO_LEFT;
      else if (!FFSTRCASECMP(FFL("stereo-right"),optarg))
        param.decode.request.channel_layout=AV_CH_STEREO_RIGHT;
      else if (!FFSTRCASECMP(FFL("wide-left"),optarg))
        param.decode.request.channel_layout=AV_CH_WIDE_LEFT;
      else if (!FFSTRCASECMP(FFL("wide-right"),optarg))
        param.decode.request.channel_layout=AV_CH_WIDE_RIGHT;
      else if (!FFSTRCASECMP(FFL("surround-direct-left"),optarg))
        param.decode.request.channel_layout=AV_CH_SURROUND_DIRECT_LEFT;
      else if (!FFSTRCASECMP(FFL("surround-direct-right"),optarg))
        param.decode.request.channel_layout=AV_CH_SURROUND_DIRECT_RIGHT;
      else if (!FFSTRCASECMP(FFL("low-frequency-2"),optarg))
        param.decode.request.channel_layout=AV_CH_LOW_FREQUENCY_2;
      else if (!FFSTRCASECMP(FFL("layout-mono"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_MONO;
      else if (!FFSTRCASECMP(FFL("layout-stereo"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_STEREO;
      else if (!FFSTRCASECMP(FFL("layout-2point1"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_2POINT1;
      else if (!FFSTRCASECMP(FFL("layout-2-1"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_2_1;
      else if (!FFSTRCASECMP(FFL("layout-surround"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_SURROUND;
      else if (!FFSTRCASECMP(FFL("layout-3point1"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_3POINT1;
      else if (!FFSTRCASECMP(FFL("layout-4point0"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_4POINT0;
      else if (!FFSTRCASECMP(FFL("layout-4point1"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_4POINT1;
      else if (!FFSTRCASECMP(FFL("layout-2-2"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_2_2;
      else if (!FFSTRCASECMP(FFL("layout-quad"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_QUAD;
      else if (!FFSTRCASECMP(FFL("layout-5point0"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_5POINT0;
      else if (!FFSTRCASECMP(FFL("layout-5point1"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_5POINT1;
      else if (!FFSTRCASECMP(FFL("layout-5point0-back"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_5POINT0_BACK;
      else if (!FFSTRCASECMP(FFL("layout-5point1-back"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_5POINT1_BACK;
      else if (!FFSTRCASECMP(FFL("layout-6point0"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_6POINT0;
      else if (!FFSTRCASECMP(FFL("layout-6point0-front"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_6POINT0_FRONT;
      else if (!FFSTRCASECMP(FFL("layout-hexagonal"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_HEXAGONAL;
      else if (!FFSTRCASECMP(FFL("layout-6point1"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_6POINT1;
      else if (!FFSTRCASECMP(FFL("layout-6point1-back"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_6POINT1_BACK;
      else if (!FFSTRCASECMP(FFL("layout_6point1_front"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_6POINT1_FRONT;
      else if (!FFSTRCASECMP(FFL("layout-7point0"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_7POINT0;
      else if (!FFSTRCASECMP(FFL("layout-7point0-front"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_7POINT0_FRONT;
      else if (!FFSTRCASECMP(FFL("layout-7point1"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_7POINT1;
      else if (!FFSTRCASECMP(FFL("layout-7point1-wide"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_7POINT1_WIDE;
      else if (!FFSTRCASECMP(FFL("layout-7point1-wide-back"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_7POINT1_WIDE_BACK;
      else if (!FFSTRCASECMP(FFL("layout-octagonal"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_OCTAGONAL;
      else if (!FFSTRCASECMP(FFL("layout-hexadecagonal"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_HEXADECAGONAL;
      else if (!FFSTRCASECMP(FFL("layout-stereo-downmix"),optarg))
        param.decode.request.channel_layout=AV_CH_LAYOUT_STEREO_DOWNMIX;
      else {
        _DMESSAGEV("argument to option --matrix not recognized:"
            " \"%" PBU_PRIs "\"",optarg);
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      break;
    case BG_ARG_STEREO:
#if defined (BG_PARAM_STEREO) // [
      param.stereo=1;
#else // ] [
      param.decode.request.channel_layout=AV_CH_LAYOUT_STEREO;
#endif // ]
      break;
    /////////////////////////////////////////////////////////////////////////
    case BG_ARG_MOMENTARY_RANGE:
      param.flags.aggregate|=BG_FLAGS_AGG_MOMENTARY_RANGE;
      break;
    ////////
    case BG_ARG_MOMENTARY_LENGTH:
      param.momentary.ms=FFATOF(optarg);
      break;
    case BG_ARG_MOMENTARY_OVERLAP:
      overlap=FFATOF(optarg);

      if (0.0<=overlap&&overlap<100.0)
        param.momentary.partition=floor(100.0/(100.0-overlap)+0.5);
      else {
        _FPRINTF(stderr,"Error: overlap out of range [0..100).\n\n");
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      break;
    case BG_ARG_MOMENTARY_MEAN_GATE:
      param.momentary.mean.gate=FFATOF(optarg);
      break;
    case BG_ARG_MOMENTARY_RANGE_GATE:
      param.momentary.range.gate=FFATOF(optarg);
      break;
    case BG_ARG_MOMENTARY_RANGE_LOWER_BOUND:
      param.momentary.range.lower_bound=FFATOF(optarg);
      break;
    case BG_ARG_MOMENTARY_RANGE_UPPER_BOUND:
      param.momentary.range.upper_bound=FFATOF(optarg);
      break;
    /////////////////////////////////////////////////////////////////////////
    case BG_ARG_SHORTTERM_MEAN:
      param.flags.aggregate|=BG_FLAGS_AGG_SHORTTERM_MEAN;
      break;
    ////////
    case BG_ARG_SHORTTERM_LENGTH:
      param.shortterm.ms=FFATOF(optarg);
      break;
    case BG_ARG_SHORTTERM_OVERLAP:
      overlap=FFATOF(optarg);

      if (0.0<=overlap&&overlap<100.0)
        param.shortterm.partition=floor(100.0/(100.0-overlap)+0.5);
      else {
        _FPRINTF(stderr,"Error: overlap out of range [0..100).\n\n");
        bg_usage(argv[0],stderr);
        goto e_arg;
      }

      break;
    case BG_ARG_SHORTTERM_MEAN_GATE:
      param.shortterm.mean.gate=FFATOF(optarg);
      break;
    case BG_ARG_SHORTTERM_RANGE_GATE:
      param.shortterm.range.gate=FFATOF(optarg);
      break;
    case BG_ARG_SHORTTERM_RANGE_LOWER_BOUND:
      param.shortterm.range.lower_bound=FFATOF(optarg);
      break;
    case BG_ARG_SHORTTERM_RANGE_UPPER_BOUND:
      param.shortterm.range.upper_bound=FFATOF(optarg);
      break;
    default:
#if defined (_WIN32) // [
      if ('c'==optopt)
        _DMESSAGEV("option -%C requires an argument",optopt);
      else if (iswprint(optopt))
        _DMESSAGEV("unknown option -%C",optopt);
      else
        _DMESSAGEV("unknown option character \\x%X",optopt);
#else // ] [
      if ('c'==optopt)
        _DMESSAGEV("option -%c requires an argument",optopt);
      else if (isprint(optopt))
        _DMESSAGEV("unknown option -%c",optopt);
      else
        _DMESSAGEV("unknown option character \\x%x",optopt);
#endif // ]

      bg_usage(argv[0],stderr);
      goto e_arg;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
#if 0 // [
  if (BG_FLAG_HELP_ARG_NULL&help_args) {
    bg_usage(argv[0],stdout);

    if (BG_FLAG_HELP_ARG_NULL==help_args) {
      code=0;
      goto e_arg;
    }
  }
#else // ] [
  if (BG_FLAG_HELP_ARG_VERSION&help_args) {
    bg_version(argv[0],stdout);
    fflush(stdout);

    if (BG_FLAG_HELP_ARG_VERSION==help_args) {
      code=0;
      goto e_arg;
    }
  }
  else if (BG_FLAG_HELP_ARG_NULL&help_args) {
    bg_usage(argv[0],stdout);
    fflush(stdout);

    if (BG_FLAG_HELP_ARG_NULL==help_args) {
      code=0;
      goto e_arg;
    }
  }
#endif // ]

#if defined (BG_PARAM_SCRIPT) // [
  if (script) {
    if (!param.output.dirname&&!param.overwrite) {
      _FPRINTF(stderr,"Error: option --script requires option -o/--output"
          " or option --overwrite.\n\n");
      bg_usage(argv[0],stderr);
      goto e_arg;
    }

    param.script=script;
    param.flags.mode|=BG_FLAGS_MODE_APPLY;
  }
#endif // ]

  // in case no class of tags is given we provide both, i.e. album and
  // track tags.
  if (!(param.flags.mode&BG_FLAGS_MODE_TAGS_ALL))
    param.flags.mode|=BG_FLAGS_MODE_TAGS_ALL;

#if defined (HAVE_FF_DYNLOAD) // [
#if defined (__linux__) && defined (__GNUC__) // [
  /////////////////////////////////////////////////////////////////////////////
  lhs=gnu_get_libc_version();
  rhs=BG_GNU_LIBC_VERSION;

  if (strcmpex(&lhs,&rhs,'.')<0||strcmpex(&lhs,&rhs,0)<0) {
#if 0 // [
    _DWARNINGV("libc might be out-dated: expecting %s, found %s",
        BG_GNU_LIBC_VERSION,gnu_get_libc_version());
    _FPRINTF(stderr,"  Might prevent loading FFmpeg shared objects."
        " Good luck!\n");
#else // ] [
    _DMESSAGEV("Attmpting to dynamically load FFmpeg shared objects by means"
        " of an out-dated libc: expecting %s, found %s",BG_GNU_LIBC_VERSION,
        gnu_get_libc_version());
    goto e_libc;
#endif // ]
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  // load the FFmpeg libraries from "bs1770gain-tools".
  if (ff_dynload(TOOLS)<0) {
    _DMESSAGE("loading shared libraries");
    goto e_dynload;
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  version=avutil_version();

  if (LIBAVUTIL_VERSION_MAJOR!=AV_VERSION_MAJOR(version)) {
#if defined (HAVE_FF_DYNLOAD) // [
    _DMESSAGEV("wrong version of avutil: expecting %d, found %d at \"%s\"",
        LIBAVUTIL_VERSION_MAJOR,AV_VERSION_MAJOR(version),ff_dynload_path());
#else // ] [
    _DMESSAGEV("wrong version of avutil: expecting %d, linked %d",
        LIBAVUTIL_VERSION_MAJOR,AV_VERSION_MAJOR(version));
#endif // ]
    goto e_versions;
  }

  ////////
  version=swresample_version();

  if (LIBSWRESAMPLE_VERSION_MAJOR!=AV_VERSION_MAJOR(version)) {
#if defined (HAVE_FF_DYNLOAD) // [
    _DMESSAGEV("wrong version of swresample: expecting %d, found %d at \"%s\"",
        LIBSWRESAMPLE_VERSION_MAJOR,AV_VERSION_MAJOR(version),
        ff_dynload_path());
#else // ] [
    _DMESSAGEV("wrong version of swresample: expecting %d, linked %d",
        LIBSWRESAMPLE_VERSION_MAJOR,AV_VERSION_MAJOR(version));
#endif // ]
    goto e_versions;
  }

  ////////
  version=avcodec_version();

  if (LIBAVCODEC_VERSION_MAJOR!=AV_VERSION_MAJOR(version)) {
#if defined (HAVE_FF_DYNLOAD) // [
    _DMESSAGEV("wrong version of avcodec: expecting %d, found %d at \"%s\"",
        LIBAVCODEC_VERSION_MAJOR,AV_VERSION_MAJOR(version),ff_dynload_path());
#else // ] [
    _DMESSAGEV("wrong version of avcodec: expecting %d, linked %d",
        LIBAVCODEC_VERSION_MAJOR,version);
#endif // ]
    goto e_versions;
  }

  ////////
  version=avformat_version();

  if (LIBAVFORMAT_VERSION_MAJOR!=AV_VERSION_MAJOR(version)) {
#if defined (HAVE_FF_DYNLOAD) // [
    _DMESSAGEV("wrong version of avformat: expecting %d, found %d at \"%s\"",
        LIBAVFORMAT_VERSION_MAJOR,AV_VERSION_MAJOR(version),ff_dynload_path());
#else // ] [
    _DMESSAGEV("wrong version of avformat: expecting %d, linked %d",
        LIBAVFORMAT_VERSION_MAJOR,AV_VERSION_MAJOR(version));
#endif // ]
    goto e_versions;
  }

  ////////
  version=swscale_version();

  if (LIBSWSCALE_VERSION_MAJOR!=AV_VERSION_MAJOR(version)) {
#if defined (HAVE_FF_DYNLOAD) // [
    _DMESSAGEV("wrong version of swscale: expecting %d, found %d at \"%s\"",
        LIBSWSCALE_VERSION_MAJOR,AV_VERSION_MAJOR(version),ff_dynload_path());
#else // ] [
    _DMESSAGEV("wrong version of swscale: expecting %d, linked %d",
        LIBSWSCALE_VERSION_MAJOR,AV_VERSION_MAJOR(version));
#endif // ]
    goto e_versions;
  }

  ////////
  version=postproc_version();

  if (LIBPOSTPROC_VERSION_MAJOR!=AV_VERSION_MAJOR(version)) {
#if defined (HAVE_FF_DYNLOAD) // [
    _DMESSAGEV("wrong version of postproc: expecting %d, found %d at \"%s\"",
        LIBPOSTPROC_VERSION_MAJOR,AV_VERSION_MAJOR(version),ff_dynload_path());
#else // ] [
    _DMESSAGEV("wrong version of postproc: expecting %d, linked %d",
        LIBPOSTPROC_VERSION_MAJOR,AV_VERSION_MAJOR(version));
#endif // ]
    goto e_versions;
  }

  ////////
  version=avfilter_version();

  if (LIBAVFILTER_VERSION_MAJOR!=AV_VERSION_MAJOR(version)) {
#if defined (HAVE_FF_DYNLOAD) // [
    _DMESSAGEV("wrong version of avfilter: %d needed, %d loaded from \"%s\"",
        LIBAVFILTER_VERSION_MAJOR,AV_VERSION_MAJOR(version),ff_dynload_path());
#else // ] [
    _DMESSAGEV("wrong version of avfilter: expecting %d, linked %d",
        LIBAVFILTER_VERSION_MAJOR,AV_VERSION_MAJOR(version));
#endif // ]
    goto e_versions;
  }

  /////////////////////////////////////////////////////////////////////////////
#if (LIBAVFORMAT_VERSION_MAJOR<58) // [
  av_register_all();
#endif // ]
#if (LIBAVFILTER_VERSION_MAJOR<7) // [
  avfilter_register_all();
#endif // ]

  if (help_args) {
    ///////////////////////////////////////////////////////////////////////////
    if (BG_FLAG_HELP_ARG_SUFFIX&help_args) {
      /////////////////////////////////////////////////////////////////////////
      _FPRINTF(stdout,"available arguments for option --suffix:\n");
#if (LIBAVFORMAT_VERSION_MAJOR<58) // [
      // https://stackoverflow.com/questions/2940521/where-to-get-full-list-of-libav-formats
      oformat=av_oformat_next(NULL);

      while (oformat) {
        if (oformat->audio_codec&&oformat->extensions)
          _FPRINTFV(stdout,"  %s\n",oformat->extensions);

        oformat=av_oformat_next(oformat);
      }
#else // ] [
      opaque=NULL;

      for (;;) {
        oformat=av_muxer_iterate(&opaque);

        if (!oformat)
          break;

        if (oformat->audio_codec&&oformat->extensions)
          _FPRINTFV(stdout,"  %s\n",oformat->extensions);
      }
#endif // ]

      fflush(stdout);
    }

    if (BG_FLAG_HELP_ARG_CODEC&help_args) {
      /////////////////////////////////////////////////////////////////////////
      _FPRINTF(stdout,"available arguments for option --codec:\n");
#if (LIBAVCODEC_VERSION_MAJOR<58) // [
      // https://stackoverflow.com/questions/2940521/where-to-get-full-list-of-libav-formats
      codec=av_codec_next(NULL);

      while (codec) {
        if (AVMEDIA_TYPE_AUDIO==codec->type)
          _FPRINTFV(stdout,"  %s\n",codec->name);

        codec=av_codec_next(codec);
      }
#else // ] [
      opaque=NULL;

      for (;;) {
        codec=av_codec_iterate(&opaque);

        if (!codec)
          break;

        if (AVMEDIA_TYPE_AUDIO==codec->type)
          _FPRINTFV(stdout,"  %s\n",codec->name);
      }
#endif // ]

      fflush(stdout);
    }

    ///////////////////////////////////////////////////////////////////////////
    code=0;
    goto e_help;
  }

#if defined (BG_PARAM_THREADS) // [
  /////////////////////////////////////////////////////////////////////////////
  if (param.nthreads<0)
    param.nthreads=0;
  else if (param.nthreads) {
    if (bg_param_threads_create(&param.threads,param.nthreads)<0) {
      _DMESSAGE("creating threads");
      goto e_threads;
    }
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
#if defined (BG_WIN32_CREATE_LOCALE) // [
  param.locale=_create_locale(LC_ALL,"");
#endif // ]
  // needed in order for _wcslwr() working as expected.
#if 1 // [
  _wsetlocale(LC_ALL,L"English_US");
#else // ] [
  _wsetlocale(LC_ALL,L"");
#endif // ]
  // if LANG is set to e.g. "en_US.UTF-8" we assume we're run from
  // e.g. MSYS2 shell undestanding UTF-8 otherwise from MS console using
  // codepage OEM. In the latter case we need an OEM representation of
  // e.g. basename.
  param.oem=!lang||!strstr(lang,"UTF-8");
#else // ] [
  // If not already active we set locale to an arbitrary UTF-8.
  locale=setlocale(LC_ALL,NULL);

  if (strstr(locale,"utf")||strstr(locale,"UTF"))
    locale=NULL;
  else
    setlocale(LC_ALL,"en_US.UTF-8");
#endif // ]


  /////////////////////////////////////////////////////////////////////////////
  if (fpath) {
#if defined (_WIN32) // [
    _wremove(fpath);

    if (utf16) {
      param.result.f=_wfopen(fpath,L"wt,ccs=UTF-16LE");
      param.print.vmt->encoding(&param,16);
    }
    else {
#if 0 // [
      param.result.f=_wfopen(fpath,L"wt");
#else // ] [
      // If the file is encoded as UTF-8, then UTF-16 data is translated
      // into UTF-8 when it is written
      param.result.f=_wfopen(fpath,L"wt,ccs=UTF-8");
#endif // ]
      param.print.vmt->encoding(&param,8);
    }
#else // ] [
    remove(fpath);
    param.result.f=fopen(fpath,"w");
    param.print.vmt->encoding(&param,8);
#endif // ]

    if (!param.result.f) {
#if defined (_WIN32) // [
      _DMESSAGEV("opening file \"%S\"",fpath);
#else // ] [
      _DMESSAGEV("opening file \"%s\"",fpath);
#endif // ]
      goto e_file;
    }
  }
#if defined (BG_PARAM_DUMP) // [
  else if (param.dump)
    param.result.f=stderr;
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  av_log_set_level(param.loglevel);

  /////////////////////////////////////////////////////////////////////////////
  if (bg_param_alloc_arguments(&param,argc-optind)<0) {
    _FPRINTF(stderr,"Error: setting arguments.\n\n");
    bg_usage(argv[0],stderr);
    goto e_arguments;
  }

  /////////////////////////////////////////////////////////////////////////////
#if defined (BG_PARAM_QUIET) // [
  verbose=!param.quiet&&!param.suppress.progress;
#else // ] [
  verbose=!param.suppress.progress;
#endif // ]

#if defined (BG_PARAM_SKIP_SCAN) // [
  if (!param.skip_scan&&verbose) {
#else // ] [
  if (verbose) {
#endif // ]
    if (&bg_print_xml_vmt==param.print.vmt)
      _FPRINTF(stdout,"<!-- ");

    _FPRINTF(stdout,"scanning ");
    fflush(stdout);
#if ! defined (BG_PARAM_SKIP_SCAN) // [
  }
#else // ] [
  }
#endif // ]

#if defined (BG_CLOCK) // [
  /////////////////////////////////////////////////////////////////////////////
  t1=clock();
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
#if defined (BG_PARAM_SKIP_SCAN) // [
  if (!param.skip_scan&&bg_param_loop(&param,argv+optind)<0) {
#else // ] [
  if (bg_param_loop(&param,argv+optind)<0) {
#endif // ]
    _DMESSAGE("counting");
    goto e_count;
#if ! defined (BG_PARAM_SKIP_SCAN) // [
  }
#else // ] [
  }
#endif // ]

#if defined (BG_PARAM_THREADS) // [
  if (0<param.nthreads)
    param.quiet=1;
#endif // ]

  if (verbose) {
    ///////////////////////////////////////////////////////////////////////////
#if defined (BG_PARAM_SKIP_SCAN) // [
    if (!param.skip_scan) {
#endif // ]
      if (&bg_print_xml_vmt==param.print.vmt)
        _FPRINTF(stdout," -->");

      _FPRINTF(stdout,"\n");
#if defined (BG_PARAM_SKIP_SCAN) // [
    }
#endif // ]

    if (&bg_print_xml_vmt==param.print.vmt)
      _FPRINTF(stdout,"<!-- ");

#if defined (BG_PARAM_THREADS) // [
    if (param.nthreads)
      _FPRINTF(stdout,"processing ...");
    else
      _FPRINTF(stdout,"analyzing ...");
#else // ] [
    _FPRINTF(stdout,"analyzing ...");
#endif // ]

    if (&bg_print_xml_vmt==param.print.vmt)
      _FPRINTF(stdout," -->");

    _FPRINTF(stdout,"\n");
    fflush(stdout);
  }

  bg_param_set_process(&param);

  if (bg_param_loop(&param,argv+optind)<0) {
    _DMESSAGE("processing");
    goto e_process;
  }

  if (verbose) {
    ///////////////////////////////////////////////////////////////////////////
    if (&bg_print_xml_vmt==param.print.vmt)
      _FPRINTF(stdout,"<!-- ");

    _FPRINTF(stdout,"done.");

    if (&bg_print_xml_vmt==param.print.vmt)
      _FPRINTF(stdout," -->");

    _FPRINTF(stdout,"\n");
    fflush(stdout);
  }

#if defined (BG_CLOCK) // [
  /////////////////////////////////////////////////////////////////////////////
  t2=clock();

  if (param.time&&!param.suppress.progress) {
    if (&bg_print_xml_vmt==param.print.vmt)
      _FPRINTF(stdout,"<!-- ");

    sec=(t2-t1)/CLOCKS_PER_SEC;

    if (sec<1.0)
      _FPRINTFV(stdout, "Duration: %.0f ms.",1000.0*sec);
    else if (sec<BG_MIN)
      _FPRINTFV(stdout, "Duration: %.3f sec.",sec);
    else if (sec<BG_HOUR)
      _FPRINTFV(stdout, "Duration: %.3f min.",sec/BG_MIN);
    else
      _FPRINTFV(stdout, "Duration: %.3f h.",sec/BG_HOUR);

    if (&bg_print_xml_vmt==param.print.vmt)
      _FPRINTF(stdout," -->");

    _FPRINTF(stdout,"\n");
    fflush(stdout);
  }
#endif // ]

  code=0;
//cleanup:
e_process:
e_count:
  bg_param_free_argumets(&param);
e_arguments:
  if (fpath)
    fclose(param.result.f);
e_file:
#if defined (_WIN32) // [
#if defined (BG_WIN32_CREATE_LOCALE) // [
  _free_locale(param.locale);
#endif // ]
#else // ] [
  if (locale)
    setlocale(LC_ALL,locale);
#endif // ]
#if defined (BG_PARAM_THREADS) // [
  bg_param_threads_destroy(&param.threads);
e_threads:
#endif // ]
e_help:
e_versions:
#if defined (HAVE_FF_DYNLOAD) // [
  ff_unload();
e_dynload:
#if defined (__linux__) && defined (__GNUC__) // [
e_libc:
#endif // ]
#endif // ]
  bg_param_destroy(&param);
e_arg:
  return code;
}
