/*
 * Berkeley Lab Checkpoint/Restart (BLCR) for Linux is Copyright (c)
 * 2008, The Regents of the University of California, through Lawrence
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
 * $Id: cr_kcompat.h,v 1.247.8.2 2009/06/12 20:37:03 phargrov Exp $
 *
 * This file tries to hide as much as practical the differences among Linux
 * kernel versions.  Preferably this is done by back-porting new features, but
 * sometimes by providing the "least common denominator"
 */

#ifndef _CR_KCOMPAT_H
#define _CR_KCOMPAT_H        1

#ifndef _IN_CR_MODULE_H
  #error "This file is only for inclusion from cr_module.h"
#endif

// Not defined in all kernels
#ifndef TASK_COMM_LEN
  #define TASK_COMM_LEN 16
#endif

// Provide uniform lockdep support
#if !defined(DECLARE_WAIT_QUEUE_HEAD_ONSTACK)
  #if defined(CONFIG_LOCKDEP)
    #define __WAIT_QUEUE_HEAD_INIT_ONSTACK(name) \
	({ init_waitqueue_head(&name); name; })
    #define DECLARE_WAIT_QUEUE_HEAD_ONSTACK(name) \
	wait_queue_head_t name = __WAIT_QUEUE_HEAD_INIT_ONSTACK(name)
  #else
    #define DECLARE_WAIT_QUEUE_HEAD_ONSTACK DECLARE_WAIT_QUEUE_HEAD
  #endif
#endif
#if !HAVE_LINUX_LOCKDEP_H
  struct lock_class_key { }; // uses no space
  #define lockdep_set_class(A,B)	do { (void)(B); } while (0)
#endif

#if defined(CONFIG_LOCKDEP)
  static __inline__ void CR_NO_LOCKS(void) {
    if (current->lockdep_depth) {
      debug_show_held_locks(current);
      dump_stack();
    }
  }
#else
  #define CR_NO_LOCKS() ((void)0)
#endif 

// MOD_{INC,DEC}_USE_COUNT were removed prior to 2.6.x
#define CR_MODULE_GET()	try_module_get(THIS_MODULE)
#define CR_MODULE_PUT()	module_put(THIS_MODULE)
#undef MOD_INC_USE_COUNT
#undef MOD_DEC_USE_COUNT
#define MOD_INC_USE_COUNT	%%%%ERROR%%%% use CR_MODULE_GET()
#define MOD_DEC_USE_COUNT	%%%%ERROR%%%% use CR_MODULE_PUT()

#ifdef CAP_KILL
  #define cr_capable(X) capable(X)
#else
  #define cr_capable(X) suser()
#endif

#if HAVE_INODE_PERMISSION
  #define cr_permission(I,M,N)        inode_permission((I),(M))
#elif HAVE_PERMISSION
  #define cr_permission(I,M,N)        permission((I),(M),(N))
#else
  #error
#endif

#if HAVE_TASK_RLIM
  #define CR_RLIM(task)	(task)->rlim
#elif HAVE_SIGNAL_RLIM
  #define CR_RLIM(task)	(task)->signal->rlim
#else
  #error
#endif

#ifndef rcu_assign_pointer
  #define rcu_assign_pointer(A,B)	((A) = (B))
#endif
#ifndef rcu_read_lock
  #define rcu_read_lock()		do {} while (0)
  #define rcu_read_unlock()		do {} while (0)
#endif

#if HAVE_STRUCT_FDTABLE
  typedef struct fdtable	cr_fdtable_t;
  #define cr_fdtable(files)	files_fdtable(files)
#else
  typedef struct files_struct	cr_fdtable_t;
  #define cr_fdtable(files)	(files)
#endif

#if HAVE_FILES_STRUCT_NEXT_FD
  #define CR_NEXT_FD(_files, _fdt) ((_files)->next_fd)
#elif HAVE_FDTABLE_NEXT_FD
  #define CR_NEXT_FD(_files, _fdt) ((_fdt)->next_fd)
#else
  #error "no next_fd"
#endif

#if HAVE_FILES_STRUCT_MAX_FDSET
  #define CR_MAX_FDS(_fdt) ((_fdt)->max_fdset)
#else
  #define CR_MAX_FDS(_fdt) ((_fdt)->max_fds)
#endif

#ifndef thread_group_leader
  #define thread_group_leader(p) ((p)->pid == (p)->tgid)
#endif

#define CR_SIGNAL_LOCK(_task)		spin_lock_irq(&(_task)->sighand->siglock)
#define CR_SIGNAL_UNLOCK(_task)		spin_unlock_irq(&(_task)->sighand->siglock)
#define CR_SIGACTION(_task,_num)	((_task)->sighand->action[(_num)-1])
#define CR_SIGNAL_HAND(_task,_num)	(CR_SIGACTION((_task),(_num)).sa.sa_handler)

#if HAVE_KMEM_CACHE_T
  typedef kmem_cache_t *cr_kmem_cache_ptr;
#else
  typedef struct kmem_cache *cr_kmem_cache_ptr;
#endif

#ifndef KMEM_CACHE
  #define KMEM_CACHE(__struct, __flags) \
        kmem_cache_create(#__struct,\
			  sizeof(struct __struct),\
			  __alignof__(struct __struct),\
			  (__flags), NULL, NULL)
#endif  

#if !HAVE_GFP_T
  typedef unsigned int gfp_t;
#endif

#if HAVE_KZALLOC
  #define cr_kzalloc kzalloc
#else
  static __inline__ void *cr_kzalloc(size_t len, gfp_t flags) {
    void *result = kmalloc(len, flags);
    if (result) {
	memset(result, 0, len);
    }
    return result;
  }
#endif

#if HAVE_KMEM_CACHE_ZALLOC
  #define cr_kmem_cache_zalloc(_szof_arg, _cachep, _flags)	\
	kmem_cache_zalloc(_cachep, _flags)
#else
  #define cr_kmem_cache_zalloc(_szof_arg, _cachep, _flags)	\
  	__cr_kmem_cache_zalloc(sizeof(_szof_arg), (_cachep), (_flags))
  static __inline__ void *__cr_kmem_cache_zalloc(size_t len, cr_kmem_cache_ptr cachep, gfp_t flags) {
    void *result = kmem_cache_alloc(cachep, flags);
    if (result) {
	memset(result, 0, len);
    }
    return result;
  }
#endif

#if HAVE_KMEMDUP
  #define cr_kmemdup kmemdup
#else
  static __inline__ void *cr_kmemdup(const void *src, size_t len, gfp_t flags) {
    void *result = kmalloc(len, flags);
    if (result) {
	memcpy(result, src, len);
    }
    return result;
  }
#endif

// Task accessor macros
#if HAVE_2_6_24_TASK_IDS
  #define cr_task_pgrp(_t)	task_pgrp_vnr(_t)
  #define cr_task_tty_old_pgrp(_t)	((_t)->signal->tty_old_pgrp)
  #define cr_task_session(_t)	task_session_vnr(_t)
  #define cr_task_leader(_t)	((_t)->signal->leader)
  #define cr_task_tty(_t)	((_t)->signal->tty)
  #if HAVE_SET_TASK_PGRP
    #define cr_set_pgrp(_t,_id)	set_task_pgrp((_t),(_id))
  #else
    #define cr_set_pgrp(_t,_id)	((void)0)
  #endif
  #if HAVE_SET_TASK_SESSION
    #define cr_set_sid(_t,_id)	set_task_session((_t),(_id))
  #else
    #define cr_set_sid(_t,_id)	((void)0)
  #endif
#elif HAVE_2_6_20_TASK_IDS
  #define cr_task_pgrp(_t)	((_t)->signal->pgrp)
  #define cr_task_tty_old_pgrp(_t)	((_t)->signal->tty_old_pgrp)
  #define cr_task_session(_t)	process_session(_t)
  #define cr_task_leader(_t)	((_t)->signal->leader)
  #define cr_task_tty(_t)	((_t)->signal->tty)
  #define cr_set_pgrp(_t,_id)	((_t)->signal->pgrp = (_id))
  #define cr_set_sid(_t,_id)	((_t)->signal->__session = (_id))
#elif HAVE_2_6_6_TASK_IDS
  #define cr_task_pgrp(_t)	((_t)->signal->pgrp)
  #define cr_task_tty_old_pgrp(_t)	((_t)->signal->tty_old_pgrp)
  #define cr_task_session(_t)	((_t)->signal->session)
  #define cr_task_leader(_t)	((_t)->signal->leader)
  #define cr_task_tty(_t)	((_t)->signal->tty)
  #define cr_set_pgrp(_t,_id)	((_t)->signal->pgrp = (_id))
  #define cr_set_sid(_t,_id)	((_t)->signal->session = (_id))
#elif HAVE_2_6_0_TASK_IDS
  #define cr_task_pgrp(_t)	((_t)->__pgrp)
  #define cr_task_tty_old_pgrp(_t)	((_t)->tty_old_pgrp)
  #define cr_task_session(_t)	((_t)->session)
  #define cr_task_leader(_t)	((_t)->leader)
  #define cr_task_tty(_t)	((_t)->tty)
  #define cr_set_pgrp(_t,_id)	((_t)->__pgrp = (_id))
  #define cr_set_sid(_t,_id)	((_t)->session = (_id))
#else
  #error
#endif

#if defined(CR_KDATA_init_pspace)
  #define cr_init_pid_ns init_pspace
#elif defined(CR_KDATA_init_pid_ns)
  #define cr_init_pid_ns init_pid_ns
#else
  // Shouldn't need it
#endif

#if HAVE_FIND_PID_NS
  // XXX: should move to vpid
  #define cr_find_pid(P) find_pid_ns((P),&cr_init_pid_ns)
#elif HAVE_1_ARG_FIND_PID
  #define cr_find_pid(P) find_pid(P)
#elif HAVE_2_ARG_FIND_PID
  #define cr_find_pid(P) find_pid(PIDTYPE_PID, (P))
#else
  #error
#endif

#if HAVE_FIND_TASK_BY_PID_NS
  // XXX: should move to by_vpid
  #define cr_find_task_by_pid(P) find_task_by_pid_ns((P),&cr_init_pid_ns)
#elif HAVE_FIND_TASK_BY_PID
  #define cr_find_task_by_pid(P) find_task_by_pid(P)
#else
  #error
#endif
/*
#if HAVE_FIND_TASK_BY_PID_TYPE_NS
  // XXX: should move to by_vpid
  #define cr_have_pid(T,P) (find_task_by_pid_type_ns((T),(P),&cr_init_pid_ns) != NULL)
#elif HAVE_FIND_TASK_BY_PID_TYPE
  #define cr_have_pid(T,P) (find_task_by_pid_type((T),(P)) != NULL)
#elif HAVE_2_ARG_FIND_PID
  #define cr_have_pid(T,P) (find_pid((T),(P)) != NULL)
#else
  #error
#endif
*/
// Process table iterators
#if HAVE_DO_EACH_PID_TASK && !HAVE_DO_EACH_TASK_PID
  #define do_each_task_pid(ID, TYPE, T)          \
    do {                                         \
      struct pid *the_pid = cr_find_pid(ID);        \
      do_each_pid_task(the_pid, (TYPE), (T))
  #define while_each_task_pid(ID, TYPE, T)       \
      while_each_pid_task(the_pid, (TYPE), (T)); \
    } while (0)
  #undef HAVE_DO_EACH_TASK_PID
  #define HAVE_DO_EACH_TASK_PID 1
#endif
#if HAVE_DO_EACH_TASK_PID
  #define CR_DO_EACH_TASK_PGID(ID, T) \
    do_each_task_pid((ID), PIDTYPE_PGID, (T))
  #define CR_WHILE_EACH_TASK_PGID(ID, T) \
    while_each_task_pid((ID), PIDTYPE_PGID, (T))

  #define CR_DO_EACH_TASK_SID(ID, T) \
    do_each_task_pid((ID), PIDTYPE_SID, (T))
  #define CR_WHILE_EACH_TASK_SID(ID, T) \
    while_each_task_pid((ID), PIDTYPE_SID, (T))

  #if HAVE_PIDTYPE_TGID
    #define CR_DO_EACH_TASK_TGID(ID, T) \
      do_each_task_pid((ID), PIDTYPE_TGID, (T))
    #define CR_WHILE_EACH_TASK_TGID(ID, T) \
      while_each_task_pid((ID), PIDTYPE_TGID, (T))
  #elif HAVE_TASK_THREAD_GROUP
    #define CR_DO_EACH_TASK_TGID(ID, T) \
      { \
	struct task_struct *_leader = cr_find_task_by_pid(ID); \
	if (_leader) { \
	  /* "extra" iteration for thread group leader (the list head): */ \
	  (T) = _leader; goto _label; \
	  list_for_each_entry((T), &(_leader->thread_group), thread_group) { \
	    _label:
    #define CR_WHILE_EACH_TASK_TGID(ID, T) \
	  } \
	} \
      }
  #else
    #error "CR_DO_EACH_TASK_TGID not implemented"
  #endif

  #define CR_DO_EACH_TASK_PROC(P, T) \
    if ((P)->mm) { \
      pid_t _id = cr_task_pgrp(P); \
      CR_DO_EACH_TASK_PGID(_id, (T)) \
        if ((T)->mm == (P)->mm)
  #define CR_WHILE_EACH_TASK_PROC(ID, T) \
      while_each_task_pid(_id, PIDTYPE_PGID, (T)); \
    }
#elif HAVE_FOR_EACH_TASK_PID
  #define CR_DO_EACH_TASK_PGID(ID, T) \
    do { \
      struct list_head *_l; \
      struct pid *_p; \
      for_each_task_pid((ID), PIDTYPE_PGID, (T), _l, _p)
  #define CR_WHILE_EACH_TASK_PGID(ID, T) \
    } while(0)

  #define CR_DO_EACH_TASK_SID(ID, T) \
    do { \
      struct list_head *_l; \
      struct pid *_p; \
      for_each_task_pid((ID), PIDTYPE_SID, (T), _l, _p)
  #define CR_WHILE_EACH_TASK_SID(ID, T) \
    } while(0)

  #define CR_DO_EACH_TASK_TGID(ID, T) \
    do { \
      struct list_head *_l; \
      struct pid *_p; \
      for_each_task_pid((ID), PIDTYPE_TGID, (T), _l, _p)
  #define CR_WHILE_EACH_TASK_TGID(ID, T) \
    } while(0)

  #define CR_DO_EACH_TASK_PROC(P, T) \
    if ((P)->mm) { \
      pid_t _id = cr_task_pgrp(P); \
      CR_DO_EACH_TASK_PGID(_id, (T)) { \
        if ((T)->mm == (P)->mm) {
  #define CR_WHILE_EACH_TASK_PROC(P, T) \
        } \
      } CR_WHILE_EACH_TASK_PGID(_id, (T)); \
    }
#else
  #error No tasklist iterators available
#endif

#define CR_DO_EACH_CHILD(C, T) \
    list_for_each_entry((C), &(T)->children, sibling) {
#define CR_WHILE_EACH_CHILD(C, T) \
    }

/* How do we manipulate the lock on a inode */
#if HAVE_INODE_SEM
  #define cr_inode_lock(_i)			down(&(_i)->i_sem)
  #define cr_inode_lock_interruptible(_i)	down_interruptible(&(_i)->i_sem)
  #define cr_inode_unlock(_i)			up(&(_i)->i_sem)
#elif  HAVE_INODE_MUTEX
  #define cr_inode_lock(_i)			mutex_lock(&(_i)->i_mutex)
  #define cr_inode_lock_interruptible(_i)	mutex_lock_interruptible(&(_i)->i_mutex)
  #define cr_inode_unlock(_i)			mutex_unlock(&(_i)->i_mutex)
#else
  #error "Unknown inode lock type"
#endif 

#ifndef wait_event_interruptible_timeout
/* from 2.6.8 */
  #define __wait_event_interruptible_timeout(wq, condition, ret) \
    do {                                                         \
      DECLARE_WAITQUEUE(__wait, current);                        \
      add_wait_queue(&(wq), &__wait);                            \
      for (;;) {                                                 \
        set_current_state(TASK_INTERRUPTIBLE);                   \
        if (condition) break;                                    \
        if (!signal_pending(current)) {                          \
	  ret = schedule_timeout(ret);                           \
	  if (!ret) break;                                       \
	  continue;                                              \
        }                                                        \
        ret = -ERESTARTSYS;                                      \
        break;                                                   \
      }                                                          \
      current->state = TASK_RUNNING;                             \
      remove_wait_queue(&(wq), &__wait);                         \
    } while(0)
  #define wait_event_interruptible_timeout(wq, condition, timeout) \
    ({ long __ret = timeout;                                       \
       if (!(condition))                                           \
         __wait_event_interruptible_timeout(wq, condition, __ret); \
       __ret;                                                      \
    })
#endif

#if defined(EXIT_ZOMBIE)
  #define cri_task_zombie(task)	 ((task)->exit_state & EXIT_ZOMBIE)
#elif defined(TASK_DEAD)
  #define cri_task_zombie(task)	 ((task)->state & TASK_ZOMBIE)
#else
  #define cri_task_zombie(task)	 ((task)->state == TASK_ZOMBIE)
#endif
#define cri_task_dead(task)	 ((task)->flags & PF_EXITING)

#ifdef DEFINE_SPINLOCK
  #define CR_DEFINE_SPINLOCK DEFINE_SPINLOCK
#else
  #define CR_DEFINE_SPINLOCK(_l) spinlock_t _l = SPIN_LOCK_UNLOCKED
#endif
#ifdef DEFINE_RWLOCK
  #define CR_DEFINE_RWLOCK DEFINE_RWLOCK
#else
  #define CR_DEFINE_RWLOCK(_l) rwlock_t _l = RW_LOCK_UNLOCKED
#endif

/* We use __putname() if we can, putname() otherwise */
#if !HAVE___PUTNAME
  #define __putname putname
#endif

/* Implement "struct path" in terms of dentry & vfsmnt */
#if !HAVE_STRUCT_PATH
  struct path {
	struct vfsmount *mnt;
	struct dentry *dentry;
  };
#endif
#if HAVE_NAMEIDATA_PATH
  static __inline__ void cr_set_pwd_file(struct fs_struct *fs, struct file *filp) {
    set_fs_pwd(fs, &filp->f_path);
  }
  static __inline__ void cr_set_pwd_nd(struct fs_struct *fs, struct nameidata *nd) {
    set_fs_pwd(fs, &nd->path);
  }
  #define nd_dentry	path.dentry
  #define nd_mnt	path.mnt
  #define cr_path_release(_nd) path_put(&((_nd)->path))
  #define CR_PATH_DECL(_name) \
	struct path *_name /* NO semicolon */
  #define CR_PATH_GET_FS(_name,_arg) \
	path_get(((_name) = &(_arg)))
  #define CR_PATH_GET_FILE(_name,_arg) \
	path_get(((_name) = &(_arg)->f_path))
#elif HAVE_NAMEIDATA_DENTRY
  static __inline__ void path_get(struct path *path) {
    mntget(path->mnt);
    dget(path->dentry);
  }
  static __inline__ void path_put(struct path *path) {
    dput(path->dentry);
    mntput(path->mnt);
  }
  static __inline__ void cr_set_pwd_file(struct fs_struct *fs, struct file *filp) {
    set_fs_pwd(fs, filp->f_vfsmnt, filp->f_dentry);
  }
  static __inline__ void cr_set_pwd_nd(struct fs_struct *fs, struct nameidata *nd) {
    set_fs_pwd(fs, nd->mnt, nd->dentry);
  }
  #define nd_dentry	dentry
  #define nd_mnt	mnt
  #define cr_path_release path_release
  #define CR_PATH_DECL(_name) \
	struct path _##_name, *_name /* NO semicolon */
  #define _CR_PATH_GET(_name,_mnt,_dentry) do { \
	_##_name.mnt = mntget(_mnt);               \
	_##_name.dentry = dget(_dentry);           \
	(_name) = &(_##_name);                     \
    } while (0)
  #define CR_PATH_GET_FS(_name,_arg) \
	_CR_PATH_GET(_name, _arg##mnt, _arg)
  #define CR_PATH_GET_FILE(_name,_arg) \
	_CR_PATH_GET(_name, (_arg)->f_vfsmnt, (_arg)->f_dentry)
#else
  #error
#endif

#if HAVE_PROC_ROOT
  #define cr_proc_root (&proc_root)
#else
  #define cr_proc_root NULL
#endif

#if HAVE_SET_DUMPABLE
  #define cr_set_dumpable(_mm,_val)	set_dumpable((_mm),(_val))
#elif HAVE_MM_DUMPABLE
  #define cr_set_dumpable(_mm,_val)	do { (_mm)->dumpable = (_val); } while (0)
#else
  #error
#endif

#if HAVE_SUID_DUMPABLE
  #define cr_suid_dumpable suid_dumpable
#else
  #define cr_suid_dumpable 0
#endif

// wait_event_timeout() first appears in 2.6.9
// This is reproduced from linux-2.6.9/include/linux/wait.h
#ifndef wait_event_timeout
  #define __wait_event_timeout(wq, condition, ret)                      \
  do {                                                                  \
        DEFINE_WAIT(__wait);                                            \
                                                                        \
        for (;;) {                                                      \
                prepare_to_wait(&wq, &__wait, TASK_UNINTERRUPTIBLE);    \
                if (condition)                                          \
                        break;                                          \
                ret = schedule_timeout(ret);                            \
                if (!ret)                                               \
                        break;                                          \
        }                                                               \
        finish_wait(&wq, &__wait);                                      \
  } while (0)

  #define wait_event_timeout(wq, condition, timeout)                    \
  ({                                                                    \
        long __ret = timeout;                                           \
        if (!(condition))                                               \
                __wait_event_timeout(wq, condition, __ret);             \
        __ret;                                                          \
  })
#endif

#if HAVE_KILL_PID
  #define cr_kill_process(task, sig)	kill_pid(task_tgid(task), sig, 0)
#elif HAVE_KILL_PROC
  #define cr_kill_process(task, sig)	kill_proc((task)->tgid, sig, 0)
#else
  #error "No cr_kill_process() definition"
#endif

#if !HAVE_VALID_SIGNAL
  static __inline__ int valid_signal(unsigned long sig) { return !!(sig <= _NSIG); }
#endif

#if HAVE_TASK_CRED
  typedef const struct cred *cr_cred_t;
  #define cr_current_cred()	current_cred()
  #define cr_task_cred(_t)	__task_cred(_t)
#else
  typedef const struct task_struct  *cr_cred_t;
  #define cr_current_cred()	current
  #define cr_task_cred(_t)	(_t)
#endif

#if defined(CR_KCODE_put_fs_struct)
  static __inline__ void cr_free_fs_struct(struct fs_struct *fs) {
    CRI_ASSERT(atomic_read(&fs->count) == 1);
    put_fs_struct(fs);
  }
#elif defined(CR_KCODE_free_fs_struct)
  #define cr_free_fs_struct free_fs_struct
#else
  #error "no cr_free_fs_struct() definition"
#endif

#if defined(CR_KCODE_do_pipe_flags)
  #define cr_do_pipe(_fds) do_pipe_flags((_fds),0)
#elif defined(CR_KCODE_do_pipe)
  #define cr_do_pipe(_fds) do_pipe(_fds)
#else
  #error "no cr_do_pipe() definition"
#endif

#endif /* _CR_KCOMPAT_H */
