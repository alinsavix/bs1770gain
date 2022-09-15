/*
 * bg_utf8_iter.c
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
#if 1 && defined (BG_UTF8_ITER) // [
#include <ctype.h>

// Iterate backwards through a utf8 multibyte string:
// https://stackoverflow.com/questions/22257486/iterate-backwards-through-a-utf8-multibyte-string

///////////////////////////////////////////////////////////////////////////////
static bg_utf8_iter_vmt_t bg_iter_utf8_le_vmt;
//#define BG_UTF8_ITER_BE
#if defined (BG_UTF8_ITER_BE) // [
static bg_utf8_iter_vmt_t bg_utf8_iter_be_vmt;
#endif // ]

///////////////////////////////////////////////////////////////////////////////
#if defined (BG_UTF8_ITER_BE) // [
static int le(void)
{
	// decide whether we're on little endian or on big endian:
  // https://www.geeksforgeeks.org/little-and-big-endian-mystery/
	// big endian is not defined yet!!!
  uint32_t i=1ul;
  const uint8_t *p=(const uint8_t *)&i;

  return *p;
}
#endif // ]

void bg_utf8_iter_first(bg_utf8_iter_t *i, const char *rp, char *wp,
		size_t *size)
{
#if defined (BG_UTF8_ITER_BE) // [
  i->vmt=le()?&bg_iter_utf8_le_vmt:&bg_utf8_iter_be_vmt;
#else // ] [
  i->vmt=&bg_iter_utf8_le_vmt;
#endif // ]
  i->rp=(const uint8_t *)rp;
  i->wp=(uint8_t *)wp;
  i->size=size;

  return;
}

///////////////////////////////////////////////////////////////////////////////
static int bg_utf8_iter_le_valid(bg_utf8_iter_t *i)
{
  // cf. e.g. "https://en.wikipedia.org/wiki/UTF-8#Description".
  const uint8_t *rp=i->rp;
  uint8_t *wp=i->mbs;

#if defined (BG_UTF8_ITER_WCS) // [
  i->wcs=0u;
#else // ] [
  //memset(wp,0,sizeof i->mbs);
#endif // ]

  if (!*i->rp)
    goto exit;
  else if (*i->rp==(BG_UTF8_ITER_MASK_ONE_BYTE&*i->rp)) {
#if 0 // [
    memcpy(wp,i->rp,1);
#else // ] [
    *wp++=*rp++;
#endif // ]
  }
  else if (*i->rp==(BG_UTF8_ITER_MASK_TWO_BYTES&*i->rp)) {
#if 0 // [
    memcpy(wp,i->rp,2);
#else // ] [
    *wp++=*rp++;
    *wp++=*rp++;
#endif // ]
  }
  else if (*i->rp==(BG_UTF8_ITER_MASK_THREE_BYTES&*i->rp)) {
#if 0 // [
    memcpy(wp,i->rp,3);
#else // ] [
    *wp++=*rp++;
    *wp++=*rp++;
    *wp++=*rp++;
#endif // ]
  }
  else if (*i->rp==(BG_UTF8_ITER_MASK_FOUR_BYTES&*i->rp)) {
#if 0 // [
    memcpy(wp,i->rp,4);
#else // ] [
    *wp++=*rp++;
    *wp++=*rp++;
    *wp++=*rp++;
    *wp++=*rp++;
#endif // ]
  }
  else
    goto exit;

  /////////////////////////////////////////////////////////////////////////////
  return 1;
exit:
  return 0;
}

#if 0 // [
static const uint8_t *bg_utf8_iter_le_next(bg_utf8_iter_t *i)
{
  // cf. e.g. "https://en.wikipedia.org/wiki/UTF-8#Description".
  if (*i->rp==(BG_UTF8_ITER_MASK_ONE_BYTE&*i->rp))
    i->rp+=1;
  else if (*i->rp==(BG_UTF8_ITER_MASK_TWO_BYTES&*i->rp))
    i->rp+=2;
  else if (*i->rp==(BG_UTF8_ITER_MASK_THREE_BYTES&*i->rp))
    i->rp+=3;
  else if (*i->rp==(BG_UTF8_ITER_MASK_FOUR_BYTES&*i->rp))
    i->rp+=4;

  return i->rp;
}
#else // ] [
static const uint8_t *bg_utf8_iter_le_next(bg_utf8_iter_t *i)
{
  uint8_t *wp=i->mbs;

  // cf. e.g. "https://en.wikipedia.org/wiki/UTF-8#Description".
  if (*i->rp==(BG_UTF8_ITER_MASK_ONE_BYTE&*i->rp))
    *wp++=*i->rp++;
  else if (*i->rp==(BG_UTF8_ITER_MASK_TWO_BYTES&*i->rp)) {
    *wp++=*i->rp++;
    *wp++=*i->rp++;
  }
  else if (*i->rp==(BG_UTF8_ITER_MASK_THREE_BYTES&*i->rp)) {
    *wp++=*i->rp++;
    *wp++=*i->rp++;
    *wp++=*i->rp++;
  }
  else if (*i->rp==(BG_UTF8_ITER_MASK_FOUR_BYTES&*i->rp)) {
    *wp++=*i->rp++;
    *wp++=*i->rp++;
    *wp++=*i->rp++;
    *wp++=*i->rp++;
  }

  return i->rp;
}
#endif // ]

static void bg_utf8_iter_le_apply(bg_utf8_iter_t *i)
{
  // cf. e.g. "https://en.wikipedia.org/wiki/UTF-8#Description".
  const uint8_t *rp=i->mbs;

  if (*rp==(BG_UTF8_ITER_MASK_ONE_BYTE&*rp)) {
    if (i->wp)
      *i->wp++=*rp++;

    if (i->size)
      *i->size+=1;
  }
  else if (*rp==(BG_UTF8_ITER_MASK_TWO_BYTES&*rp)) {
    if (i->wp) {
      memcpy(i->wp,rp,2);
      i->wp+=2;
    }

    if (i->size)
      *i->size+=2;
  }
  else if (*rp==(BG_UTF8_ITER_MASK_THREE_BYTES&*rp)) {
    if (i->wp) {
      memcpy(i->wp,rp,3);
      i->wp+=3;
    }

    if (i->size)
      *i->size+=3;
  }
  else if (*rp==(BG_UTF8_ITER_MASK_FOUR_BYTES&*rp)) {
    if (i->wp) {
      memcpy(i->wp,rp,4);
      i->wp+=4;
    }

    if (i->size)
      *i->size+=4;
  }
}

static void bg_utf8_iter_le_flush(bg_utf8_iter_t *i)
{
  if (i->wp)
    *i->wp++=0;

  if (i->size)
    ++*i->size;
}

static bg_utf8_iter_vmt_t bg_iter_utf8_le_vmt={
  .valid=bg_utf8_iter_le_valid,
  .next=bg_utf8_iter_le_next,
  .apply=bg_utf8_iter_le_apply,
  .flush=bg_utf8_iter_le_flush,
};

#if defined (BG_UTF8_ITER_BE) // [
///////////////////////////////////////////////////////////////////////////////
static int bg_utf8_iter_be_valid(bg_utf8_iter_t *i FFUNUSED)
{
  _DMESSAGE("not implemented yet");

  return 0;
}

static const uint8_t *bg_utf8_iter_be_next(bg_utf8_iter_t *i)
{
  _DMESSAGE("not implemented yet");

  return i->rp;
}

static void bg_utf8_iter_be_apply(bg_utf8_iter_t *i FFUNUSED)
{
  _DMESSAGE("not implemented yet");
}

static void bg_utf8_iter_be_flush(bg_utf8_iter_t *i FFUNUSED)
{
  _DMESSAGE("not implemented yet");
}

static bg_utf8_iter_vmt_t bg_utf8_iter_be_vmt={
  .valid=bg_utf8_iter_be_valid,
  .next=bg_utf8_iter_be_next,
  .apply=bg_utf8_iter_be_apply,
  .flush=bg_utf8_iter_be_flush,
};
#endif // ]
#endif // ]
