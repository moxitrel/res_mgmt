/* Resource management macros

* API
WITH(INIT, CHECK, EXIT) {...}
    Create resource when enter block, destroy when exit.

DEFER(EXP) {...}
    Run EXP when quit block. (golang)

RES_MGMT_REPORT()
    Print leak lines if any.

RES_MGMT_NDEBUG
    Disable leaks check when defined.

* Example
** malloc
#define WITH_MALLOC(SIZE)   WITH(void* _this = malloc(SIZE), _this != NULL, free(_this))

void WITH_MALLOC_example(void)
{
    WITH_MALLOC(sizeof(T)) {    // void* _this = malloc(sizeof(T));
                                // if (_this != NULL) {
        T* p = _this;           //      T* p = (T*)_this;
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

    DEFER(free(p)) {            //
        ...                     // ...
    }                           // free(p);
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


// generate unique var "res_mgmt_once" to avoid the warning of shadowed variable when nest WITH()
#define RES_MGMT_ONCE       RES_MGMT_ONCE_1(__LINE__)
#define RES_MGMT_ONCE_1(X)  RES_MGMT_ONCE_2(X)
#define RES_MGMT_ONCE_2(X)  res_mgmt_once_##X

// INIT : exp, run before enter block
// CHECK: exp, enter block if true
// EXIT : exp, run after exit block
//
// Use *break* to exit block.
#define WITH(INIT, CHECK, EXIT)                                                 \
    /* RES_MGMT_ONCE:           */                                              \
    /*  0: init                 */                                              \
    /* >0: create res succeed   */                                              \
    /* <0: create res failed    */                                              \
    for (int RES_MGMT_ONCE = 0; !RES_MGMT_ONCE;)                                \
                                                                                \
        /* create resource */                                                   \
        for (INIT; !RES_MGMT_ONCE;                                              \
                /* destroy resource */                                          \
                RES_MGMT_ONCE > 0 && ((void)(EXIT), RES_MGMT_LEAKS_POP()))      \
                                                                                \
            /* capture break */                                                 \
            for (;!RES_MGMT_ONCE;)                                              \
                                                                                \
                /* run block */                                                 \
                if ((CHECK)                                                     \
                    ? (RES_MGMT_ONCE++, RES_MGMT_LEAKS_PUSH(), true)            \
                    : (RES_MGMT_ONCE--, false))


#define DEFER(...)                                                              \
    for (int RES_MGMT_ONCE = 0; !RES_MGMT_ONCE; __VA_ARGS__)                    \
        /* capture break */                                                     \
        for (;!RES_MGMT_ONCE++;)


#ifndef RES_MGMT_NDEBUG
    // How many nested WITH() have leak trace
#   define RES_MGMT_LEAKS_MAX (1u << 5u)

    const char* res_mgmt_leaks[RES_MGMT_LEAKS_MAX]; // where (source line) leaks, i.e. jump out block directly
    size_t      res_mgmt_leaks_cnt;                 // how many leaks

    // print warning if leak detected
    inline static size_t res_mgmt_report(void)
    {
        const size_t max = res_mgmt_leaks_cnt < RES_MGMT_LEAKS_MAX
            ? res_mgmt_leaks_cnt
            : RES_MGMT_LEAKS_MAX;
        for (size_t i = 0; i < max; i++) {
            printf("%s\n", res_mgmt_leaks[i]);
        }
        return res_mgmt_leaks_cnt;
    }

#   define RES_MGMT_LEAKS_INFO      ("[W " __FILE__ " " TO_STRING(__LINE__) "] " "WITH() leaks!" )
#   define RES_MGMT_LEAKS_PUSH()                                                \
    (                                                                           \
        res_mgmt_leaks_cnt < RES_MGMT_LEAKS_MAX                                 \
            && (res_mgmt_leaks[res_mgmt_leaks_cnt] = RES_MGMT_LEAKS_INFO),      \
        res_mgmt_leaks_cnt++                                                    \
    )
#   define RES_MGMT_LEAKS_POP()     (res_mgmt_leaks_cnt--)
#   define RES_MGMT_REPORT()        res_mgmt_report()

#   define TO_STRING(...)           TO_STRING_(__VA_ARGS__)
#   define TO_STRING_(...)          #__VA_ARGS__
#else
#   define RES_MGMT_LEAKS_PUSH(...) 0   // nop
#   define RES_MGMT_LEAKS_POP()     0   // nop
#   define RES_MGMT_REPORT()        0   // nop
#endif

#endif // MOXITREL_RES_MGMT_H_
