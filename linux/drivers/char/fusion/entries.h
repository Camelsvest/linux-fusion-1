/*
 *	Fusion Kernel Module
 *
 *	(c) Copyright 2002-2003  Convergence GmbH
 *
 *      Written by Denis Oliver Kropp <dok@directfb.org>
 *
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef __FUSION__ENTRIES_H__
#define __FUSION__ENTRIES_H__

#include "types.h"
#include "list.h"


typedef struct __FD_FusionEntry FusionEntry;


typedef const struct {
     int object_size;

     int  (*Init)   ( FusionEntry *entry, void *ctx );
     void (*Destroy)( FusionEntry *entry, void *ctx );
} FusionEntryClass;


typedef struct {
     FusionEntryClass  *class;
     void              *ctx;

     FusionLink        *list;
     int                ids;
     struct semaphore   lock;
} FusionEntries;


struct __FD_FusionEntry {
     FusionLink         link;

     FusionEntries     *entries;

     int                id;
     pid_t              pid;

     pid_t              lock_pid;

     struct semaphore   lock;
     wait_queue_head_t  wait;
};


/* Entries Init & DeInit */

void fusion_entries_init  ( FusionEntries    *entries,
                            FusionEntryClass *class,
                            void             *ctx );

void fusion_entries_deinit( FusionEntries    *entries );


/* Create & Destroy */

int  fusion_entry_create  ( FusionEntries    *entries,
                            int              *ret_id );

int  fusion_entry_destroy ( FusionEntries    *entries,
                            int               id );


/* Lock & Unlock */

int  fusion_entry_lock    ( FusionEntries    *entries,
                            int               id,
                            FusionEntry     **ret_entry );

void fusion_entry_unlock  ( FusionEntry      *entry );


/** Wait & Notify **/

/*
 * Wait for the entry to be notified with an optional timeout.
 *
 * The entry
 *   (1) has to be locked prior to calling this function.
 *   (2) is temporarily unlocked while being waited for.
 *
 * If this function returns an error, the entry is not locked again!
 *
 * Possible errors are:
 *   -EIDRM      Entry has been removed while being waited for.
 *   -ETIMEDOUT  Timeout occured.
 *   -EINTR      A signal has been received.
 */
int  fusion_entry_wait    ( FusionEntry      *entry,
                            long             *timeout );

/*
 * Wake up one or all processes waiting for the entry to be notified.
 *
 * The entry has to be locked prior to calling this function.
 */
void fusion_entry_notify  ( FusionEntry      *entry,
                            bool              all );


#define FUSION_ENTRY_CLASS( Type, name, init_func, destroy_func )          \
                                                                           \
     static FusionEntryClass name##_class = {                              \
          .object_size = sizeof(Type),                                     \
          .Init        = init_func,                                        \
          .Destroy     = destroy_func                                      \
     };                                                                    \
                                                                           \
     static inline int fusion_##name##_lock( FusionEntries  *entries,      \
                                             int             id,           \
                                             Type          **ret_##name )  \
     {                                                                     \
          int          ret;                                                \
          FusionEntry *entry;                                              \
                                                                           \
          ret = fusion_entry_lock( entries, id, &entry );                  \
                                                                           \
          if (!ret)                                                        \
               *ret_##name = (Type *) entry;                               \
                                                                           \
          return ret;                                                      \
     }                                                                     \
                                                                           \
     static inline void fusion_##name##_unlock( Type *name )               \
     {                                                                     \
          fusion_entry_unlock( (FusionEntry*) name );                      \
     }                                                                     \
                                                                           \
     static inline int fusion_##name##_wait( Type *name, long *timeout )   \
     {                                                                     \
          return fusion_entry_wait( (FusionEntry*) name, timeout );        \
     }                                                                     \
                                                                           \
     static inline void fusion_##name##_notify( Type *name, bool all )     \
     {                                                                     \
          fusion_entry_notify( (FusionEntry*) name, all );                 \
     }


#endif
