/*
 * bg_sync.c
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

#if defined (BG_PARAM_THREADS) // [
int bg_sync_create(bg_sync_t *sync)
{
#if defined (HAVE_PTHREAD) // [
  pthread_mutexattr_t mutexattr;
  int err;

  /////////////////////////////////////////////////////////////////////////////
  memset(&mutexattr,0,sizeof mutexattr);
  err=pthread_mutexattr_settype(&mutexattr,PTHREAD_MUTEX_RECURSIVE);

  if (err<0) {
    DVMESSAGE("setting mutex attribute (%d)",err);
    goto e_mutexattr;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=pthread_mutex_init(
    &sync->mutex,   // pthread_mutex_t   *mutex,
    &mutexattr      // const pthread_mutexattr_t *mutexattr
  );

  if (err<0) {
    DVMESSAGE("creating mutex (%d)",err);
    goto e_mutex;
  }

  /////////////////////////////////////////////////////////////////////////////
  err=pthread_cond_init(
    &sync->cond,    // pthread_cond_t *cv,
    NULL            // const pthread_condattr_t *cattr
  );

  if (err<0) {
    DVMESSAGE("creating condition variable (%d)",err);
    goto e_cond;
  }
#elif defined (_WIN32) // ] [
  /////////////////////////////////////////////////////////////////////////////
  InitializeCriticalSection(&sync->cs);

  /////////////////////////////////////////////////////////////////////////////
  sync->hEvent=CreateEventW(
    NULL,           // LPSECURITY_ATTRIBUTES lpEventAttributes,
    FALSE,          // BOOL                  bManualReset,
    FALSE,          // BOOL                  bInitialState,
    NULL            // LPCSTR                lpName
  );

  if (!sync->hEvent) {
    DVMESSAGE("creating event (%lu)",GetLastError());
    goto e_event;
  }
#endif // ]

  return 0;
//cleanup:
#if defined (HAVE_PTHREAD) // [
  pthread_cond_destroy(&sync->cond);
e_cond:
  pthread_mutex_destroy(&sync->mutex);
e_mutex:
e_mutexattr:
#elif defined (_WIN32) // ] [
  CloseHandle(sync->hEvent);
e_event:
  DeleteCriticalSection(&sync->cs);
#endif // ]
  return -1;
}

void bg_sync_destroy(bg_sync_t *sync)
{
#if defined (HAVE_PTHREAD) // [
  pthread_cond_destroy(&sync->cond);
  pthread_mutex_destroy(&sync->mutex);
#elif defined (_WIN32) // ] [
  CloseHandle(sync->hEvent);
  DeleteCriticalSection(&sync->cs);
#endif // ]
}

void bg_sync_lock(bg_sync_t *sync)
{
#if defined (HAVE_PTHREAD) // [
  pthread_mutex_lock(&sync->mutex);
#elif defined (_WIN32) // ] [
  EnterCriticalSection(&sync->cs);
#endif // ]
}

void bg_sync_unlock(bg_sync_t *sync)
{
#if defined (HAVE_PTHREAD) // [
  pthread_mutex_unlock(&sync->mutex);
#elif defined (_WIN32) // ] [
  LeaveCriticalSection(&sync->cs);
#endif // ]
}

void bg_sync_signal(bg_sync_t *sync)
{
#if defined (HAVE_PTHREAD) // [
  pthread_cond_signal(&sync->cond);
#elif defined (_WIN32) // ] [
  SetEvent(sync->hEvent);
#endif // ]
}

void bg_sync_wait(bg_sync_t *sync)
{
#if defined (HAVE_PTHREAD) // [
  pthread_cond_wait(&sync->cond,&sync->mutex);
#elif defined (_WIN32) // ] [
  LeaveCriticalSection(&sync->cs);
  WaitForSingleObject(&sync->hEvent,INFINITE);
  EnterCriticalSection(&sync->cs);
#endif // ]
}
#endif // ]
