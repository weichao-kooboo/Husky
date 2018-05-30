#ifndef HKY_GCC_ATOMIC_PPC_H_INCLUDED
#define HKY_GCC_ATOMIC_PPC_H_INCLUDED

#if (HKY_PTR_SIZE == 8)

static hky_inline hky_atomic_uint_t
hky_atomic_cmp_set(hky_atomic_t *lock, hky_atomic_uint_t old,
    hky_atomic_uint_t set)
{
    hky_atomic_uint_t  res, temp;

    __asm__ volatile (

    "    li      %0, 0       \n" /* preset "0" to "res"                      */
    "    lwsync              \n" /* write barrier                            */
    "1:                      \n"
    "    ldarx   %1, 0, %2   \n" /* load from [lock] into "temp"             */
                                 /*   and store reservation                  */
    "    cmpd    %1, %3      \n" /* compare "temp" and "old"                 */
    "    bne-    2f          \n" /* not equal                                */
    "    stdcx.  %4, 0, %2   \n" /* store "set" into [lock] if reservation   */
                                 /*   is not cleared                         */
    "    bne-    1b          \n" /* the reservation was cleared              */
    "    isync               \n" /* read barrier                             */
    "    li      %0, 1       \n" /* set "1" to "res"                         */
    "2:                      \n"

    : "=&b" (res), "=&b" (temp)
    : "b" (lock), "b" (old), "b" (set)
    : "cc", "memory");

    return res;
}


static hky_inline hky_atomic_int_t
hky_atomic_fetch_add(hky_atomic_t *value, hky_atomic_int_t add)
{
    hky_atomic_uint_t  res, temp;

    __asm__ volatile (

    "    lwsync              \n" /* write barrier                            */
    "1:  ldarx   %0, 0, %2   \n" /* load from [value] into "res"             */
                                 /*   and store reservation                  */
    "    add     %1, %0, %3  \n" /* "res" + "add" store in "temp"            */
    "    stdcx.  %1, 0, %2   \n" /* store "temp" into [value] if reservation */
                                 /*   is not cleared                         */
    "    bne-    1b          \n" /* try again if reservation was cleared     */
    "    isync               \n" /* read barrier                             */

    : "=&b" (res), "=&b" (temp)
    : "b" (value), "b" (add)
    : "cc", "memory");

    return res;
}


#if (HKY_SMP)
#define hky_memory_barrier()                                                  \
    __asm__ volatile ("isync  \n  lwsync  \n" ::: "memory")
#else
#define hky_memory_barrier()   __asm__ volatile ("" ::: "memory")
#endif

#else

static hky_inline hky_atomic_uint_t
hky_atomic_cmp_set(hky_atomic_t *lock, hky_atomic_uint_t old,
    hky_atomic_uint_t set)
{
    hky_atomic_uint_t  res, temp;

    __asm__ volatile (

    "    li      %0, 0       \n" /* preset "0" to "res"                      */
    "    eieio               \n" /* write barrier                            */
    "1:                      \n"
    "    lwarx   %1, 0, %2   \n" /* load from [lock] into "temp"             */
                                 /*   and store reservation                  */
    "    cmpw    %1, %3      \n" /* compare "temp" and "old"                 */
    "    bne-    2f          \n" /* not equal                                */
    "    stwcx.  %4, 0, %2   \n" /* store "set" into [lock] if reservation   */
                                 /*   is not cleared                         */
    "    bne-    1b          \n" /* the reservation was cleared              */
    "    isync               \n" /* read barrier                             */
    "    li      %0, 1       \n" /* set "1" to "res"                         */
    "2:                      \n"

    : "=&b" (res), "=&b" (temp)
    : "b" (lock), "b" (old), "b" (set)
    : "cc", "memory");

    return res;
}


static hky_inline hky_atomic_int_t
hky_atomic_fetch_add(hky_atomic_t *value, hky_atomic_int_t add)
{
    hky_atomic_uint_t  res, temp;

    __asm__ volatile (

    "    eieio               \n" /* write barrier                            */
    "1:  lwarx   %0, 0, %2   \n" /* load from [value] into "res"             */
                                 /*   and store reservation                  */
    "    add     %1, %0, %3  \n" /* "res" + "add" store in "temp"            */
    "    stwcx.  %1, 0, %2   \n" /* store "temp" into [value] if reservation */
                                 /*   is not cleared                         */
    "    bne-    1b          \n" /* try again if reservation was cleared     */
    "    isync               \n" /* read barrier                             */

    : "=&b" (res), "=&b" (temp)
    : "b" (value), "b" (add)
    : "cc", "memory");

    return res;
}


#if (HKY_SMP)
#define hky_memory_barrier()                                                  \
    __asm__ volatile ("isync  \n  eieio  \n" ::: "memory")
#else
#define hky_memory_barrier()   __asm__ volatile ("" ::: "memory")
#endif

#endif


#define hky_cpu_pause()

#endif // HKY_GCC_ATOMIC_PPC_H_INCLUDED
