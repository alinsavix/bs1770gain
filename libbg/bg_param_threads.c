/*
 * bg_param_threads.c
 *
 * Copyright (C) 2014-2019 Peter Belkner <info@pbelkner.de>
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

#if defined (BG_PARAM_THREADS) // [
///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
static DWORD WINAPI ThreadProc(LPVOID lpParameter);
#else // ] [
static void *start_routine(void *p);
#endif // ]

#if defined (BG_PARAM_NODE_VMT) // [
static bg_param_node_vmt_t vmt;
#endif // ]

///////////////////////////////////////////////////////////////////////////////
void bg_param_request_clear(bg_param_request_t *request)
{
  request->tag=bg_param_request_tag_null;
#if defined (BG_PARAM_SCRIPT) // [
  request->script=NULL;
#endif // ]
  request->tree=NULL;
  request->visitor=NULL;
  request->dispatch=NULL;
}

///////////////////////////////////////////////////////////////////////////////
int bg_param_node_create(bg_param_node_t *node, bg_param_threads_t *threads)
{
  /////////////////////////////////////////////////////////////////////////////
#if defined (BG_PARAM_NODE_VMT) // [
  node->vmt=&vmt;
#endif // ]
  node->prev=NULL;
  node->next=NULL;
  node->threads=threads;
  bg_param_request_clear(&node->request);

  if (bg_sync_create(&node->sync)<0) {
    _DMESSAGE("creating syncition");
    goto e_sync;
  }

#if defined (_WIN32) // [
  /////////////////////////////////////////////////////////////////////////////
  node->hThread=CreateThread(
    NULL,           // LPSECURITY_ATTRIBUTES   lpThreadAttributes,
    0,              // SIZE_T                  dwStackSize,
    ThreadProc,     // LPTHREAD_START_ROUTINE  lpStartAddress,
    node,           // __drv_aliasesMem LPVOID lpParameter,
    0,              // DWORD                   dwCreationFlags,
    NULL            // LPDWORD                 lpThreadId
  );

  if (!node->hThread) {
    _DMESSAGEV("creating thread (%lu)",GetLastError());
    goto e_thread;
  }
#else // ] [
  int err;

  /////////////////////////////////////////////////////////////////////////////
  err=pthread_create(
    &node->thread,  // pthread_t *thread,
    NULL,           // const pthread_attr_t *attr,
    start_routine,  // void *(*start_routine) (void *),
    node            // void *arg
  );

  if (err) {
    _DMESSAGEV("creating thread (%d)",err);
    goto e_thread;
  }
#endif // ]

  return 0;
//cleanup:
#if ! defined (_WIN32) // [
  pthread_join(node->thread,NULL);
#else // ] [
  WaitForSingleObject(node->hThread,INFINITE);
  CloseHandle(node->hThread);
#endif // ]
e_thread:
  bg_sync_destroy(&node->sync);
e_sync:
  return -1;
}

#if 0 // [
void bg_param_node_destroy(bg_param_node_t *node, int destroy)
#else // ] [
void bg_param_node_destroy(bg_param_node_t *node)
#endif // ]
{
#if defined (BG_PARAM_SCRIPT) // [
  bg_param_node_request(node,bg_param_request_tag_kill,NULL,NULL,NULL,NULL);
#else // ] [
  bg_param_node_request(node,bg_param_request_tag_kill,NULL,NULL,NULL);
#endif // ]

#if ! defined (_WIN32) // [
  pthread_join(node->thread,NULL);
#else // ] [
  WaitForSingleObject(node->hThread,INFINITE);
  CloseHandle(node->hThread);
#endif // ]

#if 0 // [
  if (destroy)
#endif // ]
    bg_sync_destroy(&node->sync);
}

#if defined (BG_PARAM_SCRIPT) // [
void bg_param_node_request(bg_param_node_t *node, bg_param_request_tag_t tag,
    ffchar_t *script, bg_tree_t *tree, bg_visitor_t *visitor,
    bg_dispatch_t *dispatch)
#else // ] [
void bg_param_node_request(bg_param_node_t *node, bg_param_request_tag_t tag,
    bg_tree_t *tree, bg_visitor_t *visitor, bg_dispatch_t *dispatch)
#endif // ]
{
  bg_sync_lock(&node->sync); // {
  node->request.tag=tag;
#if defined (BG_PARAM_SCRIPT) // [
  node->request.script=script;
#endif // ]
  node->request.tree=tree;
  node->request.visitor=visitor;
  node->request.dispatch=dispatch;
  bg_sync_signal(&node->sync);
  bg_sync_unlock(&node->sync); // }
}

///////////////////////////////////////////////////////////////////////////////
int bg_param_list_create(bg_param_list_t *list, int n, bg_param_node_t *nodes,
    bg_param_threads_t *threads)
{
  list->count=0;
  list->head=NULL;
  list->tail=NULL;

  while (n) {
    --n;

    if (bg_param_node_create(nodes+n,threads)<0) {
      _DMESSAGE("creating node");
      goto e_node;
    }

    bg_param_list_push(list,nodes+n);
  }

  return 0;
//clean:
#if 0 // [
  bg_param_list_destroy(list,1);
#else // ] [
  bg_param_list_destroy(list);
#endif // ]
e_node:
  return -1;
}

#if 0 // [
void bg_param_list_destroy(bg_param_list_t *list, int destroy)
#else // ] [
void bg_param_list_destroy(bg_param_list_t *list)
#endif // ]
{
  bg_param_node_t *cur,*prev;

  cur=list->tail;

  while (cur) {
    prev=cur->prev;

#if 0 // [
    if (destroy)
#endif // ]
      bg_param_list_unlink(list,cur);

#if 0 // [
    bg_param_node_destroy(cur,destroy);
#else // ] [
    bg_param_node_destroy(cur);
#endif // ]
    cur=prev;
  }
}

bg_param_node_t *bg_param_list_unlink(bg_param_list_t *list,
    bg_param_node_t *node)
{
  bg_param_node_t *prev=node->prev;
  bg_param_node_t *next=node->next;

  if (prev)
    prev->next=node->next;
  else
    list->head=node->next;

  node->next=NULL;

  if (next)
    next->prev=node->prev;
  else
    list->tail=node->prev;

  node->prev=NULL;
  --list->count;

  return prev;
}

bg_param_node_t *bg_param_list_pop(bg_param_list_t *list)
{
  bg_param_node_t *node=list->tail;

  bg_param_list_unlink(list,node);

  return node;
}

bg_param_node_t *bg_param_list_push(bg_param_list_t *list,
    bg_param_node_t *node)
{
  node->prev=list->tail;
  node->next=NULL;

  if (list->tail)
    list->tail->next=node;
  else
    list->head=node;

  ++list->count;

  return list->tail=node;
}

///////////////////////////////////////////////////////////////////////////////
int bg_param_threads_create(bg_param_threads_t *threads, int n)
{
  /////////////////////////////////////////////////////////////////////////////
  if (bg_sync_create(&threads->sync)<0) {
    _DMESSAGE("creating syncition");
    goto e_sync;
  }

  /////////////////////////////////////////////////////////////////////////////
  threads->nodes=calloc(n,sizeof *threads->nodes);

  if (!threads->nodes) {
    _DMESSAGE("allocating nodes");
    goto e_nodes;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_param_list_create(&threads->list.free,n,threads->nodes,threads)<0) {
    _DMESSAGE("creating free list");
    goto e_free;
  }

  /////////////////////////////////////////////////////////////////////////////
  if (bg_param_list_create(&threads->list.active,0,NULL,threads)<0) {
    _DMESSAGE("creating active list");
    goto e_active;
  }

  /////////////////////////////////////////////////////////////////////////////
  return 0;
//cleanup:
#if 0 // [
  bg_param_list_destroy(&threads->list.active,1);
#else // ] [
  bg_param_list_destroy(&threads->list.active);
#endif // ]
e_active:
#if 0 // [
  bg_param_list_destroy(&threads->list.free,1);
#else // ] [
  bg_param_list_destroy(&threads->list.free);
#endif // ]
e_free:
  free(threads->nodes);
e_nodes:
  bg_sync_destroy(&threads->sync);
e_sync:
  return -1;
}

void bg_param_threads_destroy(bg_param_threads_t *threads FFUNUSED)
{
#if 0 // [
  bg_param_list_destroy(&threads->list.active,0);
  bg_param_list_destroy(&threads->list.free,1);
#else // ] [
  bg_param_list_destroy(&threads->list.active);
  bg_param_list_destroy(&threads->list.free);
#endif // ]
  free(threads->nodes);
  bg_sync_destroy(&threads->sync);
}

#if defined (BG_PARAM_SCRIPT) // [
void bg_param_threads_visitor_run(bg_param_threads_t *threads,
    ffchar_t *script, bg_visitor_t *visitor, bg_tree_t *tree,
    bg_dispatch_t *dispatch)
#else // ] [
void bg_param_threads_visitor_run(bg_param_threads_t *threads,
    bg_visitor_t *visitor, bg_tree_t *tree, bg_dispatch_t *dispatch)
#endif // ]
{
  bg_param_node_t *node;

  bg_sync_lock(&threads->sync); // {

#if 0 // [
  while (!threads->list.free.count) {
#else // ] [
  while (!threads->list.free.tail) {
#endif // ]
    if (!threads->list.active.count) {
      _DMESSAGE("something went wrong");
      exit(1);
    }

    bg_sync_wait(&threads->sync);
#if 1 // [
  }

  if (!threads->list.free.count) {
    _DMESSAGEV("empty free list: %d %p",threads->list.free.count,
        threads->list.free.tail);
    exit(1);
  }
#else // ] [
  }

  if (!threads->list.free.tail) {
    _DMESSAGEV("empty free list: %d %p",threads->list.free.count,
        threads->list.free.tail);
    exit(1);
  }
#endif // ]

  node=bg_param_list_pop(&threads->list.free);
  bg_param_list_push(&threads->list.active,node);

  bg_sync_unlock(&threads->sync); // }

#if defined (BG_PARAM_SCRIPT) // [
  bg_param_node_request(node,bg_param_request_tag_visitor_run,script,
      tree,visitor,dispatch);
#else // ] [
  bg_param_node_request(node,bg_param_request_tag_visitor_run,tree,visitor,
      dispatch);
#endif // ]
}

void bg_param_threads_drain(bg_param_threads_t *threads)
{
  bg_sync_lock(&threads->sync); // {

  while (threads->list.active.count)
    bg_sync_wait(&threads->sync);

  bg_sync_unlock(&threads->sync); // }
}

///////////////////////////////////////////////////////////////////////////////
void bg_param_threads_release(bg_param_threads_t *threads,
    bg_param_node_t *node)
{
#if 0 && defined (_WIN32) // [
  bg_sync_lock(&threads->sync);
#endif // ]
  bg_param_list_unlink(&threads->list.active,node);
  bg_param_list_push(&threads->list.free,node);
  bg_sync_signal(&threads->sync);
#if 0 && defined (_WIN32) // [
  bg_sync_unlock(&threads->sync);
#endif // ]
}

#if defined (BG_PARAM_NODE_VMT) // [
#define BG_PARAM_NODE_VMT_PROC 1
#else // ] [
#define BG_PARAM_NODE_VMT_PROC 0
#endif // ]

#if defined (_WIN32) // [
static DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
  bg_param_node_t *node=lpParameter;
#else // ] [
static void *start_routine(void *p)
{
  bg_param_node_t *node=p;
#endif // ]
  bg_sync_lock(&node->sync);
#if ! (BG_PARAM_NODE_VMT_PROC) // [
  bg_param_threads_t *threads=node->threads;
#endif // ]
  bg_param_request_t request;

  for (;;) {
    bg_sync_wait(&node->sync);
    request=node->request;
    bg_param_request_clear(&node->request);

    switch (request.tag) {
    case bg_param_request_tag_null:
      continue;
    case bg_param_request_tag_kill:
      goto e_kill;
    case bg_param_request_tag_visitor_run:
#if (BG_PARAM_NODE_VMT_PROC) // [
      bg_node_run(node,&request);
#else // ] [
      /////////////////////////////////////////////////////////////////////////
      request.dispatch(request.tree,request.visitor);

      /////////////////////////////////////////////////////////////////////////
      if (request.tree->parent) {
        bg_sync_lock(&request.tree->parent->helper.sync);
        --request.tree->parent->helper.nchildren;
        bg_sync_signal(&request.tree->parent->helper.sync);
        bg_sync_unlock(&request.tree->parent->helper.sync);
      }

      /////////////////////////////////////////////////////////////////////////
      bg_param_threads_release(threads,node);
#endif // ]
      continue;
    }
  }
e_kill:
  bg_sync_unlock(&node->sync);
#if ! defined (_WIN32) // [
  return NULL;
}
#else // ] [
  return 0;
}
#endif // ]


#if defined (BG_PARAM_NODE_VMT) // [
static void run(bg_param_node_t *node, bg_param_request_t *request)
{
  bg_tree_t *tree=request->tree;

#if defined (BG_PARAM_SCRIPT) // [
  if (request->script)
    bg_process_tree_run_script(tree);
  else {
#endif // ]
    ///////////////////////////////////////////////////////////////////////////
    request->dispatch(tree,request->visitor);
#if defined (BG_PARAM_SCRIPT) // [
  }
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  if (tree->parent) {
    bg_sync_lock(&tree->parent->helper.sync);
    --tree->parent->helper.nchildren;
    bg_sync_signal(&tree->parent->helper.sync);
    bg_sync_unlock(&tree->parent->helper.sync);
  }

  /////////////////////////////////////////////////////////////////////////////
  bg_param_threads_release(node->threads,node);
}

static bg_param_node_vmt_t vmt={
  .run=run,
};
#endif // ]
#endif // ]
