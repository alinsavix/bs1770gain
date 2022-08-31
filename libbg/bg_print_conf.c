/*
 * bg_print_conf.c
 *
 * Copyright (C) 2014 Peter Belkner <info@pbelkner.de>
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
#if ! defined (_WIN32) // [
#include <ctype.h>
#endif // ]

///////////////////////////////////////////////////////////////////////////////
double bg_print_conf_norm(bg_tree_t *tree)
{
  return tree->param->preamp+tree->param->norm;
}

////////
double bg_print_conf_momentary_mean(bg_tree_t *tree)
{
#if 1 // [
  return lib1770_stats_get_mean(tree->stats.momentary,
      tree->param->momentary.mean.gate);
#else // ] [
  double mean;

_WRITELNV("%p",tree);
  mean=lib1770_stats_get_mean(tree->stats.momentary,
      tree->param->momentary.mean.gate);
_WRITELNV("%p: %.02lf",tree,mean);

  return mean;
#endif // ]
}

double bg_print_conf_momentary_mean_relative(bg_tree_t *tree)
{
  double norm=bg_print_conf_norm(tree);
  double mean=bg_print_conf_momentary_mean(tree);

  return norm-mean;
}

////////
double bg_print_conf_momentary_maximum(bg_tree_t *tree)
{
  return lib1770_stats_get_max(tree->stats.momentary);
}

double bg_print_conf_momentary_maximum_relative(bg_tree_t *tree)
{
  double norm=bg_print_conf_norm(tree);
  double mean=bg_print_conf_momentary_maximum(tree);

  return norm-mean;
}

////////
double bg_print_conf_momentary_range(bg_tree_t *tree)
{
  return lib1770_stats_get_range(tree->stats.momentary,
      tree->param->momentary.range.gate,
      tree->param->momentary.range.lower_bound,
      tree->param->momentary.range.upper_bound);
}

////////
double bg_print_conf_shortterm_mean(bg_tree_t *tree)
{

  return lib1770_stats_get_mean(tree->stats.shortterm,
      tree->param->shortterm.mean.gate);
}

double bg_print_conf_shortterm_mean_relative(bg_tree_t *tree)
{
  double norm=bg_print_conf_norm(tree);
  double mean=bg_print_conf_shortterm_mean(tree);

  return norm-mean;
}

////////
double bg_print_conf_shortterm_maximum(bg_tree_t *tree)
{
  return lib1770_stats_get_max(tree->stats.shortterm);
}

double bg_print_conf_shortterm_maximum_relative(bg_tree_t *tree)
{
  double norm=bg_print_conf_norm(tree);
  double mean=bg_print_conf_shortterm_maximum(tree);

  return norm-mean;
}

////////
double bg_print_conf_shortterm_range(bg_tree_t *tree)
{
#if 1 // [
  return lib1770_stats_get_range(tree->stats.shortterm,
      tree->param->shortterm.range.gate,
      tree->param->shortterm.range.lower_bound,
      tree->param->shortterm.range.upper_bound);
#else // ] [
  double range;

DWRITELN("===");
  range=lib1770_stats_get_range(tree->stats.shortterm,
      tree->param->shortterm.range.gate,
      tree->param->shortterm.range.lower_bound,
      tree->param->shortterm.range.upper_bound);
DVWRITELN("=== range:%.02f",range);

  return range;
#endif // ]
}

////////
double bg_print_conf_samplepeak_absolute(bg_tree_t *tree)
{
  return tree->stats.samplepeak;
}

double bg_print_conf_samplepeak_relative(bg_tree_t *tree)
{
  return LIB1770_Q2DB(tree->stats.samplepeak);
}

////////
double bg_print_conf_truepeak_absolute(bg_tree_t *tree)
{
  return tree->stats.truepeak;
}

double bg_print_conf_truepeak_relative(bg_tree_t *tree)
{
  return LIB1770_Q2DB(tree->stats.truepeak);
}

///////////////////////////////////////////////////////////////////////////////
const char *bg_print_conf_unit_lum(bg_tree_t *tree)
{
  return tree->param->unit->n.lu;
}

#if defined (_WIN32) // [
const wchar_t *bg_print_conf_unit_luw(bg_tree_t *tree)
{
  return tree->param->unit->w.lu;
}
#endif // ]

////////
const char *bg_print_conf_unit_lram(bg_tree_t *tree)
{
  return tree->param->unit->n.lra;
}

#if defined (_WIN32) // [
const wchar_t *bg_print_conf_unit_lraw(bg_tree_t *tree)
{
  return tree->param->unit->w.lra;
}
#endif // ]

////////
const char *bg_print_conf_unit_spm(bg_tree_t *tree)
{
  return tree->param->unit->n.sp;
}

#if defined (_WIN32) // [
const wchar_t *bg_print_conf_unit_spw(bg_tree_t *tree)
{
  return tree->param->unit->w.sp;
}
#endif // ]

////////
const char *bg_print_conf_unit_tpm(bg_tree_t *tree)
{
  return tree->param->unit->n.tp;
}

#if defined (_WIN32) // [
const wchar_t *bg_print_conf_unit_tpw(bg_tree_t *tree)
{
  return tree->param->unit->w.tp;
}
#endif // ]

////////
#if defined (_WIN32) // [
#define BG_CONF(AGG,LABEL1,LABEL2,LABEL3,ARGC,FN1,FORMAT1,FN2,FORMAT2) { \
  .agg=AGG, \
  .w.label={ .classic=L##LABEL1, .xml=L##LABEL2, .csv=L##LABEL3 }, \
  .n.label={ .classic=LABEL1, .xml=LABEL2, .csv=LABEL3 }, \
  .argc=ARGC, \
  \
  .argv={ \
    { .fn=FN1, .w={ .format=L##FORMAT1 }, .n={ .format=FORMAT1 } }, \
    { .fn=FN2, .w={ .format=L##FORMAT2 }, .n={ .format=FORMAT2 } }, \
  } \
}
#else // ] [
#define BG_CONF(AGG,LABEL1,LABEL2,LABEL3,ARGC,FN1,FORMAT1,FN2,FORMAT2) { \
  .agg=AGG, \
  .n.label={ .classic=LABEL1, .xml=LABEL2, .csv=LABEL3 }, \
  .argc=ARGC, \
  \
  .argv={ \
    { .fn=FN1, .n={ .format=FORMAT1 } }, \
    { .fn=FN2, .n={ .format=FORMAT2 } }, \
  } \
}
#endif // ]

#define BG_CONF1(AGG,LABEL1,LABEL2,LABEL3,FN1,FORMAT1) \
    BG_CONF(AGG,LABEL1,LABEL2,LABEL3,1,FN1,FORMAT1,NULL,"")
#define BG_CONF2(AGG,LABEL1,LABEL2,LABEL3,FN1,FORMAT1,FN2,FORMAT2) \
    BG_CONF(AGG,LABEL1,LABEL2,LABEL3,2,FN1,FORMAT1,FN2,FORMAT2)

bg_print_conf_t bg_print_conf[BG_FLAGS_AGG_MAX_OFFSET]={
  BG_CONF2(BG_FLAGS_AGG_MOMENTARY_MEAN,
      "integrated (momentary mean)",
      "integrated",
      "integrated",
      bg_print_conf_momentary_mean,
      "%.2f",
      bg_print_conf_momentary_mean_relative,
      "%.2f"),
  BG_CONF2(BG_FLAGS_AGG_MOMENTARY_MAXIMUM,
      "momentary maximum",
      "momentary-maximum",
      "momentary maximum",
      bg_print_conf_momentary_maximum,
      "%.2f",
      bg_print_conf_momentary_maximum_relative,
      "%.2f"),
  BG_CONF1(BG_FLAGS_AGG_MOMENTARY_RANGE,
#if defined (BG_UNIT_LRA) // [
      "momentary loudness range",
      "momentary-range",
      "momentary range",
#else // ] [
      "momentary lra",
      "momentary-lra",
      "momentary lra",
#endif // ]
      bg_print_conf_momentary_range,
      "%.2f"),
  BG_CONF2(BG_FLAGS_AGG_SHORTTERM_MEAN,
      "shortterm mean",
      "shortterm-mean",
      "shortterm mean",
      bg_print_conf_shortterm_mean,
      "%.2f",
      bg_print_conf_shortterm_mean_relative,
      "%.2f"),
  BG_CONF2(BG_FLAGS_AGG_SHORTTERM_MAXIMUM,
      "shortterm maximum",
      "shortterm-maximum",
      "shortterm maximum",
      bg_print_conf_shortterm_maximum,
      "%.2f",
      bg_print_conf_shortterm_maximum_relative,
      "%.2f"),
  BG_CONF1(BG_FLAGS_AGG_SHORTTERM_RANGE,
#if defined (BG_UNIT_LRA) // [
      "shortterm loudness range",
      "shortterm-range",
      "shortterm range",
#else // ] [
      "shortterm lra",
      "shortterm-lra",
      "shortterm lra",
#endif // ]
      bg_print_conf_shortterm_range,
#if 1 // [
      "%.2f"
#else
      "%f"
#endif // ]
      ),
  BG_CONF2(BG_FLAGS_AGG_SAMPLEPEAK,
      "sample peak",
      "sample-peak",
      "sample peak",
      bg_print_conf_samplepeak_relative,
      "%.2f",
      bg_print_conf_samplepeak_absolute,
      "%f"),
  BG_CONF2(BG_FLAGS_AGG_TRUEPEAK,
      "true peak",
      "true-peak",
      "true peak",
      bg_print_conf_truepeak_relative,
      "%.2f",
      bg_print_conf_truepeak_absolute,
      "%f"),
};

#if defined (_WIN32) // [
void bg_print_conf_unitw(FILE *f, int lc FFUNUSED, bg_print_conf_t *c,
    bg_param_t *param, int argv)
{
  const wchar_t *unit;

  if (stdout==f||stderr==f) {
    DMESSAGE("writing to consule");
    return;
  }

  switch (c-bg_print_conf) {
  case BG_FLAGS_AGG_MOMENTARY_MEAN_OFFSET:
    unit=param->unit->w.lu;
    break;
  case BG_FLAGS_AGG_MOMENTARY_MAXIMUM_OFFSET:
    unit=param->unit->w.lu;
    break;
  case BG_FLAGS_AGG_MOMENTARY_RANGE_OFFSET:
    unit=param->unit->w.lra;
    break;
  case BG_FLAGS_AGG_SHORTTERM_MEAN_OFFSET:
    unit=param->unit->w.lu;
    break;
  case BG_FLAGS_AGG_SHORTTERM_MAXIMUM_OFFSET:
    unit=param->unit->w.lu;
    break;
  case BG_FLAGS_AGG_SHORTTERM_RANGE_OFFSET:
    unit=param->unit->w.lra;
    break;
  case BG_FLAGS_AGG_SAMPLEPEAK_OFFSET:
    unit=param->unit->w.sp;
    break;
  case BG_FLAGS_AGG_TRUEPEAK_OFFSET:
    unit=param->unit->w.tp;
    break;
  default:
    DMESSAGE("out of range");
    return;
  }

  switch (c-bg_print_conf) {
  case BG_FLAGS_AGG_MOMENTARY_MEAN_OFFSET:
  case BG_FLAGS_AGG_MOMENTARY_MAXIMUM_OFFSET:
  case BG_FLAGS_AGG_SHORTTERM_MEAN_OFFSET:
  case BG_FLAGS_AGG_SHORTTERM_MAXIMUM_OFFSET:
    if (lc) {
      while (*unit)
        fputwc(towlower(*unit++),f);

      if (!argv)
        fputws(L"fs",f);
    }
    else {
      fputws(unit,f);

      if (!argv)
        fputws(L"FS",f);
    }

    break;
  case BG_FLAGS_AGG_MOMENTARY_RANGE_OFFSET:
  case BG_FLAGS_AGG_SHORTTERM_RANGE_OFFSET:
    if (lc) {
      while (*unit)
        fputwc(towlower(*unit++),f);
    }
    else
      fputws(unit,f);

    break;
  case BG_FLAGS_AGG_SAMPLEPEAK_OFFSET:
  case BG_FLAGS_AGG_TRUEPEAK_OFFSET:
    if (0<argv)
      fputws(L"amplitude",f);
    else if (lc) {
      while (*unit)
        fputwc(towlower(*unit++),f);

      fputws(L"fs",f);
    }
    else {
      fputws(unit,f);
      fputws(L"FS",f);
    }

    break;
  default:
    DMESSAGE("out of range");
    return;
  }
}
#endif // ]

void bg_print_conf_unit(FILE *f, int lc, bg_print_conf_t *c,
    bg_param_t *param, int argv)
{
  const char *unit;

  switch (c-bg_print_conf) {
  case BG_FLAGS_AGG_MOMENTARY_MEAN_OFFSET:
    unit=param->unit->n.lu;
    break;
  case BG_FLAGS_AGG_MOMENTARY_MAXIMUM_OFFSET:
    unit=param->unit->n.lu;
    break;
  case BG_FLAGS_AGG_MOMENTARY_RANGE_OFFSET:
    unit=param->unit->n.lra;
    break;
  case BG_FLAGS_AGG_SHORTTERM_MEAN_OFFSET:
    unit=param->unit->n.lu;
    break;
  case BG_FLAGS_AGG_SHORTTERM_MAXIMUM_OFFSET:
    unit=param->unit->n.lu;
    break;
  case BG_FLAGS_AGG_SHORTTERM_RANGE_OFFSET:
    unit=param->unit->n.lra;
    break;
  case BG_FLAGS_AGG_SAMPLEPEAK_OFFSET:
    unit=param->unit->n.sp;
    break;
  case BG_FLAGS_AGG_TRUEPEAK_OFFSET:
    unit=param->unit->n.tp;
    break;
  default:
    DMESSAGE("out of range");
    return;
  }

  switch (c-bg_print_conf) {
  case BG_FLAGS_AGG_MOMENTARY_MEAN_OFFSET:
  case BG_FLAGS_AGG_MOMENTARY_MAXIMUM_OFFSET:
  case BG_FLAGS_AGG_SHORTTERM_MEAN_OFFSET:
  case BG_FLAGS_AGG_SHORTTERM_MAXIMUM_OFFSET:
    if (lc) {
      while (*unit)
        fputc(tolower(*unit++),f);

      if (!argv)
        fputs("fs",f);
    }
    else {
      fputs(unit,f);

      if (!argv)
        fputs("FS",f);
    }

    break;
  case BG_FLAGS_AGG_MOMENTARY_RANGE_OFFSET:
  case BG_FLAGS_AGG_SHORTTERM_RANGE_OFFSET:
    if (lc) {
      while (*unit)
        fputc(tolower(*unit++),f);
    }
    else
      fputs(unit,f);

    break;
  case BG_FLAGS_AGG_SAMPLEPEAK_OFFSET:
  case BG_FLAGS_AGG_TRUEPEAK_OFFSET:
    if (0<argv)
      fputs("amplitude",f);
    else if (lc) {
      while (*unit)
        fputc(tolower(*unit++),f);

      fputs("fs",f);
    }
    else {
      fputs(unit,f);
      fputs("FS",f);
    }

    break;
  default:
    DMESSAGE("out of range");
    return;
  }
}
