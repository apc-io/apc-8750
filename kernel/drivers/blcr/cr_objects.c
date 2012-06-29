/*
 * Berkeley Lab Checkpoint/Restart (BLCR) for Linux is Copyright (c)
 * 2003, The Regents of the University of California, through Lawrence
 * Berkeley National Laboratory (subject to receipt of any required
 * approvals from the U.S. Dept. of Energy).  All rights reserved.
 *
 * Portions may be copyrighted by others, as may be noted in specific
 * copyright notices within specific files.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: cr_objects.c,v 1.11 2008/08/21 05:22:49 phargrov Exp $
 */

#include "cr_module.h"

#define MAP_SIZE 64
#define MAP_SHIFT 6

/* doubly linked list of key=val pairs */
struct cr_objectmap_pair { /* No "_s" suffix to fit kmem_cache naming requirements */
    struct list_head	list;
    void *key;
    void *val; 
};

/* Map is an array of lock+list pairs */
struct cr_objectmap_s {
    struct cr_objectmap_entry_s {
	rwlock_t		lock;
	struct list_head	list;
    } table[MAP_SIZE];
};

static cr_kmem_cache_ptr cr_objmap_cachep = NULL;
static cr_kmem_cache_ptr cr_object_cachep = NULL;

int
cr_object_init(void)
{
	cr_objmap_cachep = KMEM_CACHE(cr_objectmap_s, 0);
	if (!cr_objmap_cachep) goto no_objmap_cachep;
	cr_object_cachep = KMEM_CACHE(cr_objectmap_pair, 0);
	if (!cr_object_cachep) goto no_object_cachep;
	return 0;

no_object_cachep:
	kmem_cache_destroy(cr_objmap_cachep);
no_objmap_cachep:
	return -ENOMEM;
}

void
cr_object_cleanup(void)
{
	if (cr_objmap_cachep) kmem_cache_destroy(cr_objmap_cachep);
	if (cr_object_cachep) kmem_cache_destroy(cr_object_cachep);
}

static
int hash_it(void *x)
{
    unsigned long tmp = (unsigned long)x;
    tmp = tmp ^ (tmp >> MAP_SHIFT) ^ (tmp >> (MAP_SHIFT*2));
    return (int)(tmp % MAP_SIZE);
}

cr_objectmap_t
cr_alloc_objectmap(void)
{
    static struct lock_class_key lock_key;
    struct cr_objectmap_s *map = kmem_cache_alloc(cr_objmap_cachep, GFP_KERNEL);

    if (map) {
	int i;
	struct cr_objectmap_entry_s *entry;
	for (i=0, entry=&map->table[0]; i<MAP_SIZE; ++i, ++entry) {
	    rwlock_init(&(entry->lock));
	    lockdep_set_class(&(entry->lock), &lock_key);
	    INIT_LIST_HEAD(&(entry->list));
	} 
    }

    return map;
}

void
cr_release_objectmap(cr_objectmap_t map)
{
    int i;
    struct cr_objectmap_entry_s *entry;

    for (i=0, entry=&map->table[0]; i<MAP_SIZE; ++i, ++entry) {
	struct cr_objectmap_pair *pair, *next;
	list_for_each_entry_safe(pair, next, &(entry->list), list) {
	    kmem_cache_free(cr_object_cachep, pair);
	}
    }
    kmem_cache_free(cr_objmap_cachep, map);
}

/*
 * returns int rather than void * to allow NULL key's to be placed into table
 *
 * 1 - found
 * 0 - not found (but *val_p still may be written)
 */
int
cr_find_object(cr_objectmap_t map, void *key, void **val_p)
{
    int retval = 0;

    /* result is NULL by default, use return value to distinguish good NULL from evil NULL */
    if (val_p != NULL) {
	*val_p = NULL;
    }

    if (key == NULL) {
    	/* special cased to avoid confusion with not present */
        // CR_KTRACE_LOW_LVL("map %p: Asked for NULL, returning NULL.", map);
        retval = 1;
    } else {
	int h = hash_it(key);
	struct cr_objectmap_entry_s *entry = &map->table[h];
	struct cr_objectmap_pair *pair;

        read_lock(&(entry->lock));
	list_for_each_entry(pair, &(entry->list), list) {
	    if (pair->key == key) {
		// CR_KTRACE_LOW_LVL("map %p: Found object %p in slot %d", map, key, h);
		if (val_p != NULL)
		    *val_p = pair->val;
		retval = 1; 
		break;
	    } else if (pair->key > key) {
		/* Sorted order would have placed pair here */
		break;
	    }
	}
        read_unlock(&(entry->lock));

	// if (!retval) CR_KTRACE_LOW_LVL("map %p: Object %p not found", map, key);
    }

    return retval;
}

/*
 *  1 if it's in there already 
 *  0 if we insert it
 */
int
cr_insert_object(cr_objectmap_t map, void *key, void *val, gfp_t flags)
{
    struct cr_objectmap_pair *new_pair, *pair;
    int retval = -1;
    int h = hash_it(key);
    struct cr_objectmap_entry_s *entry = &map->table[h];

    /* If not GFP_ATOMIC, we'd better not hold any locks */
    if (flags != GFP_ATOMIC) CR_NO_LOCKS();

    /* special case NULL -> NULL */
    if (key == NULL) {
        retval = 1;
	return retval;
    }

    /* assume it's not there yet to move alloc outside of lock and keep a single pass */
    new_pair = kmem_cache_alloc(cr_object_cachep, flags);
    new_pair->key = key;
    new_pair->val = val;

    write_lock(&(entry->lock));
    list_for_each_entry(pair, &(entry->list), list) {
	if (pair->key == key) {
	    /* return 1 if it's in there already */
	    // CR_KTRACE_LOW_LVL("map %p: Object %p already inserted into slot %d", map, key, h);
	    retval = 1; 
	    break;
	} else if (pair->key > key) {
	    /* Sorted order places new pair here */
	    break;
	}
    }
    if (retval != 1) {
	// CR_KTRACE_LOW_LVL("map %p: Inserting object %p into slot %d", map, key, h);
	list_add_tail(&new_pair->list, &pair->list);
	retval = 0;
    }
    write_unlock(&(entry->lock));

    if (retval) {
	kmem_cache_free(cr_object_cachep, new_pair);
    }

    return retval;
}

/*
 *  0 if we remove it
 *  -1 if it's not in there
 */
int
cr_remove_object(cr_objectmap_t map, void *key)
{
    struct cr_objectmap_pair *pair, *next;
    struct cr_objectmap_entry_s *entry = &map->table[hash_it(key)];
    int retval=-1;

    write_lock(&(entry->lock));
    list_for_each_entry_safe(pair, next, &(entry->list), list) {
	if (pair->key == key) {
	    list_del(&pair->list);
	    kmem_cache_free(cr_object_cachep, pair);
	    retval = 1; 
	    break;
	}
    }
    write_unlock(&(entry->lock));

    return retval;
}
