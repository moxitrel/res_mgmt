/* Resource management macros

* API
WITH(INIT, CHECK, EXIT) {...}
    for like syntax. Create resource when enter block, destroy when exit.

DEFER(EXP) {...}
    Run EXP when quit block. (golang)

RES_MGMT_CHECK()
    Print leaks info for WITH().

RES_MGMT_NDEBUG
    Disable leaks check when defined.

* Example
** malloc
#define WITH_MALLOC(SIZE)   WITH(T* _this = malloc(SIZE), _this != NULL, free(_this))

void WITH_MALLOC_example(void)
{
    WITH_MALLOC(sizeof(T)) {    // T* _this = malloc(sizeof(T));
                                // if (_this != NULL) {
        ...                     //      ...
                                //      free(_this);
    }                           // }
}

** FILE
#define WITH_FILE(FILENAME, MODE)   WITH(FILE* _this = fopen(FILENAME, MODE), _this != NULL, fclose(_this))

void WITH_FILE_example(void)
{
    WITH_FILE("file.txt", "r") {    // FILE* _this = fopen("file.txt", "r");
                                    // if (_this != NULL) {
        ...                         //      ...
                                    //      fclose(_this);
    }                               // }
}

** malloc
void DEFER_example(void)
{
    T* p = malloc(sizeof *p);
    if (p == NULL) {
        return;
    }

    DEFER(free(p)) {    //
        ...             // ...
    }                   // free(p);
}

* Idea
for (T* p = malloc(sizeof *p); p != NULL; free(p), p = NULL)
{
    //
    // user code
    //
}

* Drawbacks
- Must not *jump out* block directly, so don't use: return, goto or longjmp() (out of block)

*/
#ifndef MOXITREL_RES_MGMT_H_
#define MOXITREL_RES_MGMT_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

// INIT : exp, run before enter block
// CHECK: exp, enter block if true
// EXIT : exp, run after exit block
//
// Use break to quit block.
#define WITH(INIT, CHECK, EXIT)                                                     \
    /* res_mgmt_once:           */                                                  \
    /*  0: init                 */                                                  \
    /* >0: create res succeed   */                                                  \
    /* <0: create res failed    */                                                  \
    for (int res_mgmt_once = 0; !res_mgmt_once;)                                    \
                                                                                    \
        /* create resource */                                                       \
        for (INIT; !res_mgmt_once;                                                  \
                /* destroy resource */                                              \
                res_mgmt_once > 0 && ((void)(EXIT), RES_MGMT_LEAKS_POP()))          \
                                                                                    \
            /* capture break */                                                     \
            for (;!res_mgmt_once;)                                                  \
                                                                                    \
                /* run block */                                                     \
                if ((CHECK)                                                         \
                    ? (++res_mgmt_once, RES_MGMT_LEAKS_PUSH(), true)                \
                    : (--res_mgmt_once, false))


#define DEFER(...)                                                      \
    for (int res_mgmt_once = 0; !res_mgmt_once; __VA_ARGS__)            \
        /* capture break */                                             \
        for (;!res_mgmt_once++;)


#ifndef RES_MGMT_NDEBUG
    // How many nested WITH() with leaks trace. The value should be exp2(x)
#   define RES_MGMT_LEAKS_MAX (1 << 5)

    const char* res_mgmt_leaks[RES_MGMT_LEAKS_MAX]; // where (source line) leaks, i.e. jump out block directly
    size_t      res_mgmt_leaks_cnt;                 // how many leaks

    // print warning if leak detect
    inline static size_t res_mgmt_leaks_check(void)
    {
        size_t max = res_mgmt_leaks_cnt < RES_MGMT_LEAKS_MAX ? res_mgmt_leaks_cnt : RES_MGMT_LEAKS_MAX;
        for (size_t i = 0; i < max; i++) {
            printf("%s\n", res_mgmt_leaks[i]);
        }
        return res_mgmt_leaks_cnt;
    }

#   define RES_MGMT_LEAKS_INFO()    ("WITH() leaks at: " __FILE__ " " TO_STRING(__LINE__))
#   define RES_MGMT_LEAKS_PUSH()                                                                \
    (                                                                                           \
        assert(res_mgmt_leaks_cnt < RES_MGMT_LEAKS_MAX),                                        \
        /* use & instead of % */                                                                \
        res_mgmt_leaks[res_mgmt_leaks_cnt & (RES_MGMT_LEAKS_MAX-1)] = RES_MGMT_LEAKS_INFO(),    \
        res_mgmt_leaks_cnt++                                                                    \
    )
#   define RES_MGMT_LEAKS_POP()     (res_mgmt_leaks_cnt--)
#   define RES_MGMT_CHECK()         res_mgmt_leaks_check()

#   define TO_STRING(...)           TO_STRING_(__VA_ARGS__)
#   define TO_STRING_(...)          #__VA_ARGS__
#else
#   define RES_MGMT_LEAKS_PUSH(...) 0   // nop
#   define RES_MGMT_LEAKS_POP()     0   // nop
#   define RES_MGMT_CHECK()         0   // nop
#endif

#endif // MOXITREL_RES_MGMT_H_
