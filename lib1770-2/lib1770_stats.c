/*
 * lib1770_stats.c
 * Copyright (C) 2014 Peter Belkner <info@pbelkner.de>
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
#include <pbutil_priv.h>

//#define FF_STATS_EXPERIMENTAL

#if ! defined (LIB1770_HIST_NBINS) // [
int lib1770_stats_create(lib1770_stats_t *stats)
{
  double step=1.0/LIB1770_HIST_GRAIN;
  lib1770_bin_t *wp,*mp;

#if 0 // [
  stats=LIB1770_CALLOC(1,(sizeof *stats)
      +LIB1770_HIST_NBINS*(sizeof stats->hist.bin[0]));
  LIB1770_GOTO(NULL==stats,"allocating bs.1770 statistics",stats);
#endif // ]

///////////////////////////////////////////////////////////////////////////////
  stats->max.wmsq=LIB1770_SILENCE_GATE;

///////////////////////////////////////////////////////////////////////////////
  stats->hist.pass1.wmsq=0.0;
  stats->hist.pass1.count=0;

  wp=stats->hist.bin;
  mp=wp+LIB1770_HIST_NBINS;

  while (wp<mp) {
    size_t i=wp-stats->hist.bin;
    double db=step*i+LIB1770_HIST_MIN;
    double wsmq=pow(10.0,0.1*(0.691+db));

    wp->db=db;
    wp->x=wsmq;
    wp->y=0.0;
    wp->count=0;

    if (0<i)
      wp[-1].y=wsmq;

    ++wp;
  }

  return 0;
#if 0 // [
stats:
  return NULL;
#endif // ]
}

void lib1770_stats_destroy(lib1770_stats_t *stats)
{
  (void)stats;
}
#endif // ]

lib1770_stats_t *lib1770_stats_new(void)
{
  lib1770_stats_t *stats;
#if defined (LIB1770_HIST_NBINS) // [
  double step=1.0/LIB1770_HIST_GRAIN;
  lib1770_bin_t *wp,*mp;
#endif // ]

  stats=LIB1770_CALLOC(1,(sizeof *stats)
      +LIB1770_HIST_NBINS*(sizeof stats->hist.bin[0]));
  LIB1770_GOTO(NULL==stats,"allocating bs.1770 statistics",stats);

#if defined (LIB1770_HIST_NBINS) // [
///////////////////////////////////////////////////////////////////////////////
  stats->max.wmsq=LIB1770_SILENCE_GATE;

///////////////////////////////////////////////////////////////////////////////
  stats->hist.pass1.wmsq=0.0;
  stats->hist.pass1.count=0;

  wp=stats->hist.bin;
  mp=wp+LIB1770_HIST_NBINS;

  while (wp<mp) {
    size_t i=wp-stats->hist.bin;
    double db=step*i+LIB1770_HIST_MIN;
    double wsmq=pow(10.0,0.1*(0.691+db));

    wp->db=db;
    wp->x=wsmq;
    wp->y=0.0;
    wp->count=0;

    if (0<i)
      wp[-1].y=wsmq;

    ++wp;
  }

  return stats;
#else // ] [
  if (lib1770_stats_create(stats)<0)
    goto create;

  return stats;
create:
  LIB1770_FREE(stats);
#endif // ]
stats:
  return NULL;
}

void lib1770_stats_close(lib1770_stats_t *stats)
{
#if ! defined (LIB1770_HIST_NBINS) // [
  lib1770_stats_destroy(stats);
#endif // ]
  LIB1770_FREE(stats);
}

#if defined (LIB1770_STATS_MERGE_FIX) // [
void lib1770_stats_merge(lib1770_stats_t *lhs, const lib1770_stats_t *rhs)
#else // ] [
void lib1770_stats_merge(lib1770_stats_t *lhs, lib1770_stats_t *rhs)
#endif // ]
{
  // m1=(sum_n c_n)/n
  // m2=(sum_m c_m)/m
  //
  // n*m1=sum_n c_n
  // m*m2=sum_m c_m
  //
  // ((sum_n c_n) + (sum_m c_n)) / (m+n)
  //     = n/(m+n) m1 + m/(m+n) m2
  lib1770_count_t count;
  double q1,q2;
#if defined (LIB1770_STATS_MERGE_FIX) // [
  lib1770_bin_t *bin1,*mp;
  const lib1770_bin_t *bin2;
#else // ] [
  lib1770_bin_t *bin1,*bin2,*mp;
#endif // ]

  if (lhs->max.wmsq<rhs->max.wmsq)
    lhs->max.wmsq=rhs->max.wmsq;

  count=lhs->hist.pass1.count+rhs->hist.pass1.count;

  if (0ull<count) {
    q1=(double)lhs->hist.pass1.count/count;
    q2=(double)rhs->hist.pass1.count/count;
#if defined (LIB1770_STATS_MERGE_FIX) // [
    lhs->hist.pass1.count=count;
#endif // ]
    lhs->hist.pass1.wmsq=q1*lhs->hist.pass1.wmsq+q2*rhs->hist.pass1.wmsq;
    bin1=lhs->hist.bin;
    bin2=rhs->hist.bin;
    mp=bin1+LIB1770_HIST_NBINS;

    while (bin1<mp)
      (bin1++)->count+=(bin2++)->count;
  }
}

static int lib1770_bin_cmp(const void *key, const void *bin)
{
  if (*(const double *)key<((const lib1770_bin_t *)bin)->x)
    return -1;
  else if (0==((const lib1770_bin_t *)bin)->y)
    return 0;
  else if (((const lib1770_bin_t *)bin)->y<=*(const double *)key)
    return 1;
  else
    return 0;
}

void lib1770_stats_add_sqs(lib1770_stats_t *stats, double wmsq)
{
  lib1770_bin_t *bin;

///////////////////////////////////////////////////////////////////////////////
  if (stats->max.wmsq<wmsq)
    stats->max.wmsq=wmsq;

///////////////////////////////////////////////////////////////////////////////
  bin=bsearch(&wmsq,stats->hist.bin,LIB1770_HIST_NBINS,
      sizeof stats->hist.bin[0],lib1770_bin_cmp);

  if (NULL!=bin) {
    // cumulative moving average.
    // https://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
#if 1 // {
  //                       x (n) - CMA (n)
  // CMA (n+1) = CMA (n) + ---------------
  //                          n + 1
    stats->hist.pass1.wmsq+=(wmsq-stats->hist.pass1.wmsq)
        /(double)(++stats->hist.pass1.count);
#else // } {
  //             x + n * CMA (n)
    // CMA (n+1) = ---------------
  //                n + 1
  //
  //           = x / (n + 1) + CMA (n) * n / (n+1)
  //
  double n=stats->hist.pass1.count;
  double m=n+1;

    stats->hist.pass1.wmsq*=n/m;
    stats->hist.pass1.wmsq+=wmsq/m;
  ++stats->hist.pass1.count;
#endif // }
    ++bin->count;
  }
}

///////////////////////////////////////////////////////////////////////////////
double lib1770_stats_get_max(lib1770_stats_t *stats)
{
  return LIB1770_LUFS(stats->max.wmsq);
}

///////////////////////////////////////////////////////////////////////////////
#if defined (FF_STATS_EXPERIMENTAL) // [
#define FF_CMP_REF
#endif // ]
#if defined (FF_CMP_REF) // [
typedef struct cmp_ref cmp_ref_t;

struct cmp_ref {
  int (*cmp)(cmp_ref_t *ref, double x);
  double gate1; // in amplitude, i.e. *not* dB
  double gate2; // in amplitude, i.e. *not* dB
};

static int cmp1(cmp_ref_t *ref, double x)
{
  return ref->gate1<x;
}

//#if 0 // [
static int cmp2(cmp_ref_t *ref, double x)
{
  return ref->gate1<x&&ref->gate2<x;
}
//#endif // ]

double lib1770_stats_get_mean_ex(lib1770_stats_t *stats, cmp_ref_t *ref,
    lib1770_count_t *count)
#else // ] [
double lib1770_stats_get_mean_ex(lib1770_stats_t *stats, double gate,
    lib1770_count_t *count)
#endif // ]
{
  const lib1770_bin_t *rp,*mp;
  double wmsq=0.0;

  *count=0ull;
  rp=stats->hist.bin;
  mp=rp+LIB1770_HIST_NBINS;
#if ! defined (FF_CMP_REF) // [
  gate=stats->hist.pass1.wmsq*pow(10,0.1*gate);
#endif // ]

  while (rp<mp) {
#if defined (FF_CMP_REF) // [
    if (0ull<rp->count&&ref->cmp(ref,rp->x)) {
#else // ] [
    if (0ull<rp->count&&gate<rp->x) {
#endif // ]
      wmsq+=(double)rp->count*rp->x;
      *count+=rp->count;
#if ! defined (FF_CMP_REF) // [
    }
#else // ] [
    }
#endif // ]

    ++rp;
  }

  return LIB1770_LUFS_HIST(*count,wmsq,LIB1770_SILENCE);
}

double lib1770_stats_get_mean(lib1770_stats_t *stats, double gate)
{
#if 0 // [
  const lib1770_bin_t *rp,*mp;
  double wmsq=0.0;
  lib1770_count_t count=0ull;

  rp=stats->hist.bin;
  mp=rp+LIB1770_HIST_NBINS;
  gate=stats->hist.pass1.wmsq*pow(10,0.1*gate);

  while (rp<mp) {
    if (0ull<rp->count&&gate<rp->x) {
      wmsq+=(double)rp->count*rp->x;
      count+=rp->count;
    }

    ++rp;
  }

  return LIB1770_LUFS_HIST(count,wmsq,LIB1770_SILENCE);
#else // ] [
  lib1770_count_t count;
#if defined (FF_CMP_REF) // [
  cmp_ref_t ref
      ={  .cmp=cmp1, .gate1=stats->hist.pass1.wmsq*pow(10,0.1*gate) };

  return lib1770_stats_get_mean_ex(stats,&ref,&count);
#else // ] [
  return lib1770_stats_get_mean_ex(stats,gate,&count);
#endif // ]

#endif // ]
}

#if ! defined (FF_STATS_EXPERIMENTAL) // [
double lib1770_stats_get_range(lib1770_stats_t *stats, double gate,
    double lower, double upper)
{
  const lib1770_bin_t *rp=stats->hist.bin;
  const lib1770_bin_t *mp=rp+LIB1770_HIST_NBINS;
  // count of the hits to the current bin of histogram (n):
  lib1770_count_t count=0ull;

  // transform gate from dB to amplitude.
  gate=stats->hist.pass1.wmsq*pow(10,0.1*gate);

  // how many hits we have to count?
  while (rp<mp) {
    if (gate<rp->x&&0ull<rp->count)
      count+=rp->count;

    ++rp;
  }

  // if necessary, swap boundaries.
  if (lower>upper) {
    double tmp=lower;

    lower=upper;
    upper=tmp;
  }

  if (lower<0.0)
    lower=0.0;

  if (1.0<upper)
    upper=1.0;

  if (0ull<count) {
    lib1770_count_t lower_count=count*lower;
    lib1770_count_t upper_count=count*upper;
    lib1770_count_t prev_count=-1;
    double min=NAN;
    double max=NAN;

    count=0ull;

    // for all bins:
    for (rp=stats->hist.bin;rp<mp;++rp) {
      //  only include loudness levels that are above gate threshold
      if (gate<rp->x/*&&rp->x<db*/) {
        count+=rp->count;

        // if not alrady done, properly initialize min and max.
        if (isnan(min)||isnan(max)) {
          min=rp->db;
          max=rp->db;
          prev_count=count;
          continue;
        }

        if (prev_count<lower_count&&lower_count<=count)
          min=rp->db;

        if (prev_count<upper_count&&upper_count<=count) {
          max=rp->db;
          break;
        }

        prev_count=count;
      }
    }

    return isnan(max)||isnan(min)?0.0:max-min;
  }
  else
    return 0.0;
}
#elif 1 // ] [
#define LIB1770_STATS_RANGE_AMPLITUDE
#define LIB1770_STATS_RANGE_RECALC
double lib1770_stats_get_range(lib1770_stats_t *stats, double gate,
    double lower, double upper)
{
  // TODO
  const lib1770_bin_t *rp=stats->hist.bin;
  const lib1770_bin_t *mp=rp+LIB1770_HIST_NBINS;
  // count of the hits to the current bin of histogram (n):
  lib1770_count_t count=0ull;
  cmp_ref_t ref;

  ref.gate1=stats->hist.pass1.wmsq*pow(10,0.1*-70);
  ref.cmp=cmp1;
  ref.gate2=lib1770_stats_get_mean_ex(stats,&ref,&count);
  ref.gate2=stats->hist.pass1.wmsq*pow(10,0.1*ref.gate2-20.0);
  ref.cmp=cmp2;
  gate=lib1770_stats_get_mean_ex(stats,&ref,&count);
  // transform gate from dB to amplitude.
  gate=stats->hist.pass1.wmsq*pow(10,0.1*gate);
DVWRITELN("gate:%.02f",gate);

  // how many hits we have to count?
  while (rp<mp) {
    if (gate<rp->x&&0ull<rp->count)
      count+=rp->count;

    ++rp;
  }

  // if necessary, swap boundaries.
  if (lower>upper) {
    double tmp=lower;

    lower=upper;
    upper=tmp;
  }

  if (lower<0.0)
    lower=0.0;

  if (1.0<upper)
    upper=1.0;

  if (0ull<count) {
    lib1770_count_t lower_count=count*lower;
    lib1770_count_t upper_count=count*upper;
    lib1770_count_t prev_count=-1;
    double min=NAN;
    double max=NAN;

    count=0ull;

    // for all bins:
    for (rp=stats->hist.bin;rp<mp;++rp) {
      //  only include loudness levels that are above gate threshold
      if (ref.cmp(&ref,rp->x)) {
        count+=rp->count;

        // if not alrady done, properly initialize min and max.
        if (isnan(min)||isnan(max)) {
          min=rp->db;
          max=rp->db;
          prev_count=count;
          continue;
        }

        if (prev_count<lower_count&&lower_count<=count)
          min=rp->db;

        if (prev_count<upper_count&&upper_count<=count) {
          max=rp->db;
          break;
        }

        prev_count=count;
      }
    }

    return isnan(max)||isnan(min)?0.0:max-min;
  }
  else
    return 0.0;
}
#elif 1 // ] [
#define LIB1770_STATS_RANGE_AMPLITUDE
#define LIB1770_STATS_RANGE_RECALC
double lib1770_stats_get_range(lib1770_stats_t *stats, double gate,
    double lower, double upper)
{
  const lib1770_bin_t *rp=stats->hist.bin;
  const lib1770_bin_t *mp=rp+LIB1770_HIST_NBINS;
  // count of the hits to the current bin of histogram (n):
  lib1770_count_t count=0ull;
#if defined (FF_CMP_REF) // [
#if defined (LIB1770_STATS_RANGE_AMPLITUDE) // ] [
  cmp_ref_t ref_static
      ={ .cmp=cmp1, .gate1=stats->hist.pass1.wmsq*pow(10,0.1*gate) };
#else // ] [
  cmp_ref_t ref_static
      ={ .cmp=cmp1, .gate1=gate };
#endif // ]
#endif // ]

DVWRITELN("\ngate:%.02f",gate);
#if defined (LIB1770_STATS_RANGE_RECALC) // [
  gate=lib1770_stats_get_mean_ex(stats,&ref_static,&count);
DVWRITELN("gate:%.02f",gate);
#endif // ]

#if defined (LIB1770_STATS_RANGE_AMPLITUDE) // [
  // transform gate from dB to amplitude.
  gate=stats->hist.pass1.wmsq*pow(10,0.1*gate);
DVWRITELN("gate:%.02f",gate);
#endif // ]

  // how many hits we have to count?
  while (rp<mp) {
    if (gate<rp->x&&0ull<rp->count)
      count+=rp->count;

    ++rp;
  }

  // if necessary, swap boundaries.
  if (lower>upper) {
    double tmp=lower;

    lower=upper;
    upper=tmp;
  }

  if (lower<0.0)
    lower=0.0;

  if (1.0<upper)
    upper=1.0;

  if (0ull<count) {
    lib1770_count_t lower_count=count*lower;
    lib1770_count_t upper_count=count*upper;
    lib1770_count_t prev_count=-1;
    double min=NAN;
    double max=NAN;

    count=0ull;

    // for all bins:
    for (rp=stats->hist.bin;rp<mp;++rp) {
      //  only include loudness levels that are above gate threshold
#if defined (LIB1770_STATS_RANGE_AMPLITUDE) // [
      if (gate<rp->x) {
#else // ] [
      if (gate<rp->db) {
#endif // ]
        count+=rp->count;

        // if not alrady done, properly initialize min and max.
        if (isnan(min)||isnan(max)) {
          min=rp->db;
          max=rp->db;
          prev_count=count;
          continue;
        }

        if (prev_count<lower_count&&lower_count<=count)
          min=rp->db;

        if (prev_count<upper_count&&upper_count<=count) {
          max=rp->db;
          break;
        }

        prev_count=count;
      }
#if ! defined (LIB1770_STATS_RANGE_AMPLITUDE) // [
    }
#else // ] [
    }
#endif // ]

    return isnan(max)||isnan(min)?0.0:max-min;
  }
  else
    return 0.0;
}
#else // ] [
double x(lib1770_stats_t *stats, double gate, lib1770_count_t *count,
    double *min, double *max)
{
  const lib1770_bin_t *mp=stats->hist.bin+LIB1770_HIST_NBINS;
  const lib1770_bin_t *rp;

  *count=0ull;
  *min=NAN;
  *max=NAN;

  for (rp=stats->hist.bin;rp<mp;++rp) {
    //  only include loudness levels that are above gate threshold
    if (gate<rp->x) {
      // if not alrady done, properly initialize min and max.
      if (isnan(*min)||isnan(*max)) {
        *min=rp->db;
        *max=rp->db;
        //prev_count=count;
        continue;
      }

      if (rp->db<*min)
        *min=rp->db;

      if (*max<rp->db)
        *max=rp->db;
//DVWRITELN("??? %f:%f %f",rp->db,*min,*max);
    }
  }

//DVWRITELN("!!! %f:%f %f",*max-*min,*min,*max);
    return isnan(*max)||isnan(*min)?0.0:*max-*min;
}

double lib1770_stats_get_range(lib1770_stats_t *stats, double gate,
    double lower, double upper)
{
#if 0 // [
  const lib1770_bin_t *rp=stats->hist.bin;
  const lib1770_bin_t *mp=rp+LIB1770_HIST_NBINS;
  // count of the hits in current bin of histogram:
  lib1770_count_t count=0ull;
  double db=0.0;
#endif // ]

#if 0 // [
  // transform gate from dB to amplitude.
  gate=stats->hist.pass1.wmsq*pow(10,0.1*gate);

  // how many hits we have to count?
  while (rp<mp) {
    if (gate<rp->x&&0ull<rp->count) {
      db+=rp->db;
      count+=rp->count;
    }

    ++rp;
  }

  db=LIB1770_Q2DB(LIB1770_DB2Q(db)/count);

  // if necessary, swap boundaries.
  if (lower>upper) {
    double tmp=lower;

    lower=upper;
    upper=tmp;
  }

  if (lower<0.0)
    lower=0.0;

  if (1.0<upper)
    upper=1.0;

  if (0ull<count) {
    //lib1770_count_t lower_count=count*lower;
    //lib1770_count_t upper_count=count*upper;
    //lib1770_count_t prev_count=-1;
    double min=NAN;
    double max=NAN;

    count=0ull;

    // for all bins:
    for (rp=stats->hist.bin;rp<mp;++rp) {
      //  only include loudness levels that are above gate threshold
      if (gate<rp->x) {
        //count+=rp->count;

        // if not alrady done, properly initialize min and max.
        if (isnan(min)||isnan(max)) {
          min=rp->db;
          max=rp->db;
          //prev_count=count;
          continue;
        }

#if 0 // [
        if (prev_count<lower_count&&lower_count<=count)
          min=rp->db;

        if (prev_count<upper_count&&upper_count<=count) {
          max=rp->db;
          break;
        }

        prev_count=count;
#else // ] [
        if (rp->db<min)
          min=rp->db;

        if (max<rp->db)
          max=rp->db;
DVWRITELN("??? %f:%f %f",rp->db,min,max);
#endif // ]
      }
    }

DVWRITELN("!!! %f:%f %f",max-min,min,max);
    return isnan(max)||isnan(min)?0.0:max-min;
  }
  else
    return 0.0;
#else // ] [
  const lib1770_bin_t *rp=stats->hist.bin;
  const lib1770_bin_t *mp=rp+LIB1770_HIST_NBINS;
    lib1770_count_t count;
    double abs_gate,rel_gate;
    //double min,max;

    (void)lower;
    (void)upper;
//DVWRITELN("\nlower:%.02f",lower);
//DVWRITELN("\nupper:%.02f",upper);
//DVWRITELN("\ngate:%.02f",gate);
//exit(1);
    gate=-70;
    abs_gate=lib1770_stats_get_mean_ex(stats,gate,&count);
    abs_gate=LIB1770_Q2DB(LIB1770_DB2Q(abs_gate)/count)-20.0;
    rel_gate=lib1770_stats_get_mean_ex(stats,abs_gate,&count);
    //rel_gate=LIB1770_Q2DB(LIB1770_DB2Q(rel_gate)/count);
//DVWRITELN("=== abs_gate:%.02f rel_gate:%.02f",abs_gate,rel_gate);

  // how many hits we have to count?
  rp=stats->hist.bin;
  count=0;

  while (rp<mp) {
    if ((abs_gate<rp->x&&rel_gate<rp->x))
      count+=rp->count;

    ++rp;
  }

  if (0ull<count) {
    lib1770_count_t lower_count=count*lower;
    lib1770_count_t upper_count=count*upper;
    lib1770_count_t prev_count=-1;
    double min=NAN;
    double max=NAN;

    count=0ull;

    // for all bins:
    for (rp=stats->hist.bin;rp<mp;++rp) {
      //  only include loudness levels that are above gate threshold
      if (abs_gate<rp->db&&rel_gate<rp->db) {
        count+=rp->count;

        // if not alrady done, properly initialize min and max.
        if (isnan(min)||isnan(max)) {
          min=rp->db;
          max=rp->db;
          prev_count=count;
//DVWRITELN("\n??? min:%f:%f:%d max:%f",min,rel_gate,rel_gate<rp->db,max);
          continue;
        }

#if 1 // [
        if (prev_count<lower_count&&lower_count<=count)
          min=rp->db;

        if (prev_count<upper_count&&upper_count<=count) {
          max=rp->db;
          break;
        }

        prev_count=count;
#else // ] [
        if (rp->db<min)
          min=rp->db;

        if (max<rp->db)
          max=rp->db;
//DVWRITELN("??? %f:%f %f",rp->db,min,max);
#endif // ]
      }
    }

//DVWRITELN("!!! %f:%f (rel_gate:%f) %f",max-min,min,rel_gate,max);
    return isnan(max)||isnan(min)?0.0:max-min;
  }
  else
    return 0.0;
#endif // ]
}
#endif // ]
