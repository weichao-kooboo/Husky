#ifndef HKY_ATOMIC_H_INCLUDED
#define HKY_ATOMIC_H_INCLUDED

#include "../core/hky_config.h"
#include "../core/hky_core.h"

#if (HKY_HAVE_LIBATOMIC)

#define AO_REQUIRE_CAS
#include <atomic_ops.h>

#define HKY_HAVE_ATOMIC_OPS  1

typedef long                        hky_atomic_int_t;
typedef AO_t                        hky_atomic_uint_t;
typedef volatile hky_atomic_uint_t  hky_atomic_t;

#if (HKY_PTR_SIZE == 8)
#define HKY_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#else
#define HKY_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#endif

#define hky_atomic_cmp_set(lock, old, new)                                    \
    AO_compare_and_swap(lock, old, new)
#define hky_atomic_fetch_add(value, add)                                      \
    AO_fetch_and_add(value, add)
#define hky_memory_barrier()        AO_nop()
#define hky_cpu_pause()


#elif (HKY_DARWIN_ATOMIC)

/*
 * use Darwin 8 atomic(3) and barrier(3) operations
 * optimized at run-time for UP and SMP
 */

#include <libkern/OSAtomic.h>

/* "bool" conflicts with perl's CORE/handy.h */
#if 0
#undef bool
#endif


#define HKY_HAVE_ATOMIC_OPS  1

#if (HKY_PTR_SIZE == 8)

typedef int64_t                     hky_atomic_int_t;
typedef uint64_t                    hky_atomic_uint_t;
#define HKY_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)

#define hky_atomic_cmp_set(lock, old, new)                                    \
    OSAtomicCompareAndSwap64Barrier(old, new, (int64_t *) lock)

#define hky_atomic_fetch_add(value, add)                                      \
    (OSAtomicAdd64(add, (int64_t *) value) - add)

#else

typedef int32_t                     hky_atomic_int_t;
typedef uint32_t                    hky_atomic_uint_t;
#define HKY_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)

#define hky_atomic_cmp_set(lock, old, new)                                    \
    OSAtomicCompareAndSwap32Barrier(old, new, (int32_t *) lock)

#define hky_atomic_fetch_add(value, add)                                      \
    (OSAtomicAdd32(add, (int32_t *) value) - add)

#endif

#define hky_memory_barrier()        OSMemoryBarrier()

#define hky_cpu_pause()

typedef volatile hky_atomic_uint_t  hky_atomic_t;


#elif (HKY_HAVE_GCC_ATOMIC)

/* GCC 4.1 builtin atomic operations */

#define HKY_HAVE_ATOMIC_OPS  1

typedef long                        hky_atomic_int_t;
typedef unsigned long               hky_atomic_uint_t;

#if (HKY_PTR_SIZE == 8)
#define HKY_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#else
#define HKY_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#endif

typedef volatile hky_atomic_uint_t  hky_atomic_t;


#define hky_atomic_cmp_set(lock, old, set)                                    \
    __sync_bool_compare_and_swap(lock, old, set)

#define hky_atomic_fetch_add(value, add)                                      \
    __sync_fetch_and_add(value, add)

#define hky_memory_barrier()        __sync_synchronize()

#if ( __i386__ || __i386 || __amd64__ || __amd64 )
#define hky_cpu_pause()             __asm__ ("pause")
#else
#define hky_cpu_pause()
#endif


#elif ( __i386__ || __i386 )

typedef int32_t                     hky_atomic_int_t;
typedef uint32_t                    hky_atomic_uint_t;
typedef volatile hky_atomic_uint_t  hky_atomic_t;
#define HKY_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)


#if ( __SUNPRO_C )

#define HKY_HAVE_ATOMIC_OPS  1

hky_atomic_uint_t
hky_atomic_cmp_set(hky_atomic_t *lock, hky_atomic_uint_t old,
    hky_atomic_uint_t set);

hky_atomic_int_t
hky_atomic_fetch_add(hky_atomic_t *value, hky_atomic_int_t add);

/*
 * Sun Studio 12 exits with segmentation fault on '__asm ("pause")',
 * so hky_cpu_pause is declared in src/os/unix/hky_sunpro_x86.il
 */

void
hky_cpu_pause(void);

/* the code in src/os/unix/hky_sunpro_x86.il */

#define hky_memory_barrier()        __asm (".volatile"); __asm (".nonvolatile")


#else /* ( __GNUC__ || __INTEL_COMPILER ) */

#define HKY_HAVE_ATOMIC_OPS  1

#include "hky_gcc_atomic_x86.h"

#endif


#elif ( __amd64__ || __amd64 )

typedef int64_t                     hky_atomic_int_t;
typedef uint64_t                    hky_atomic_uint_t;
typedef volatile hky_atomic_uint_t  hky_atomic_t;
#define HKY_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)


#if ( __SUNPRO_C )

#define HKY_HAVE_ATOMIC_OPS  1

hky_atomic_uint_t
hky_atomic_cmp_set(hky_atomic_t *lock, hky_atomic_uint_t old,
    hky_atomic_uint_t set);

hky_atomic_int_t
hky_atomic_fetch_add(hky_atomic_t *value, hky_atomic_int_t add);

/*
 * Sun Studio 12 exits with segmentation fault on '__asm ("pause")',
 * so hky_cpu_pause is declared in src/os/unix/hky_sunpro_amd64.il
 */

void
hky_cpu_pause(void);

/* the code in src/os/unix/hky_sunpro_amd64.il */

#define hky_memory_barrier()        __asm (".volatile"); __asm (".nonvolatile")


#else /* ( __GNUC__ || __INTEL_COMPILER ) */

#define HKY_HAVE_ATOMIC_OPS  1

#include "hky_gcc_atomic_amd64.h"

#endif


#elif ( __sparc__ || __sparc || __sparcv9 )

#if (HKY_PTR_SIZE == 8)

typedef int64_t                     hky_atomic_int_t;
typedef uint64_t                    hky_atomic_uint_t;
#define HKY_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)

#else

typedef int32_t                     hky_atomic_int_t;
typedef uint32_t                    hky_atomic_uint_t;
#define HKY_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)

#endif

typedef volatile hky_atomic_uint_t  hky_atomic_t;


#if ( __SUNPRO_C )

#define HKY_HAVE_ATOMIC_OPS  1

#include "hky_sunpro_atomic_sparc64.h"


#else /* ( __GNUC__ || __INTEL_COMPILER ) */

#define HKY_HAVE_ATOMIC_OPS  1

#include "hky_gcc_atomic_sparc64.h"

#endif


#elif ( __powerpc__ || __POWERPC__ )

#define HKY_HAVE_ATOMIC_OPS  1

#if (HKY_PTR_SIZE == 8)

typedef int64_t                     hky_atomic_int_t;
typedef uint64_t                    hky_atomic_uint_t;
#define HKY_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)

#else

typedef int32_t                     hky_atomic_int_t;
typedef uint32_t                    hky_atomic_uint_t;
#define HKY_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)

#endif

typedef volatile hky_atomic_uint_t  hky_atomic_t;


#include "hky_gcc_atomic_ppc.h"

#endif


#if !(HKY_HAVE_ATOMIC_OPS)

#define HKY_HAVE_ATOMIC_OPS  0

typedef int32_t                     hky_atomic_int_t;
typedef uint32_t                    hky_atomic_uint_t;
typedef volatile hky_atomic_uint_t  hky_atomic_t;
#define HKY_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)


static hky_inline hky_atomic_uint_t
hky_atomic_cmp_set(hky_atomic_t *lock, hky_atomic_uint_t old,
    hky_atomic_uint_t set)
{
    if (*lock == old) {
        *lock = set;
        return 1;
    }

    return 0;
}


static hky_inline hky_atomic_int_t
hky_atomic_fetch_add(hky_atomic_t *value, hky_atomic_int_t add)
{
    hky_atomic_int_t  old;

    old = *value;
    *value += add;

    return old;
}

#define hky_memory_barrier()
#define hky_cpu_pause()

#endif


void hky_spinlock(hky_atomic_t *lock, hky_atomic_int_t value, hky_uint_t spin);

#define hky_trylock(lock)  (*(lock) == 0 && hky_atomic_cmp_set(lock, 0, 1))
#define hky_unlock(lock)    *(lock) = 0

#endif // HKY_ATOMIC_H_INCLUDED
