#ifndef HKY_GCC_ATOMIC_SPARC64_H_INCLUDED
#define HKY_GCC_ATOMIC_SPARC64_H_INCLUDED

#if (HKY_PTR_SIZE == 4)
#define HKY_CASA  "casa"
#else
#define HKY_CASA  "casxa"
#endif


static hky_inline hky_atomic_uint_t
hky_atomic_cmp_set(hky_atomic_t *lock, hky_atomic_uint_t old,
    hky_atomic_uint_t set)
{
    __asm__ volatile (

    HKY_CASA " [%1] 0x80, %2, %0"

    : "+r" (set) : "r" (lock), "r" (old) : "memory");

    return (set == old);
}


static hky_inline hky_atomic_int_t
hky_atomic_fetch_add(hky_atomic_t *value, hky_atomic_int_t add)
{
    hky_atomic_uint_t  old, res;

    old = *value;

    for ( ;; ) {

        res = old + add;

        __asm__ volatile (

        HKY_CASA " [%1] 0x80, %2, %0"

        : "+r" (res) : "r" (value), "r" (old) : "memory");

        if (res == old) {
            return res;
        }

        old = res;
    }
}


#if (HKY_SMP)
#define hky_memory_barrier()                                                  \
            __asm__ volatile (                                                \
            "membar #LoadLoad | #LoadStore | #StoreStore | #StoreLoad"        \
            ::: "memory")
#else
#define hky_memory_barrier()   __asm__ volatile ("" ::: "memory")
#endif

#define hky_cpu_pause()

#endif // HKY_GCC_ATOMIC_SPARC64_H_INCLUDED
