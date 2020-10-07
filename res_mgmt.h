/* Resource management macros

* API
WITH(ENTER, CHECK, EXIT) {...}
    Create resource when enter block, destroy when exit.

DEFER(EXP) {...}
    Run EXP when quit block. (golang)

RES_MGMT_LEAKS()
    Return leak locations. (const char* [])

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

// generate unique var "res_mgmt_once" to avoid the warning of shadowed variable when nest WITH()
#define RES_MGMT_ONCE       RES_MGMT_ONCE0(__LINE__)
#define RES_MGMT_ONCE0(X)   RES_MGMT_ONCE1(X)
#define RES_MGMT_ONCE1(X)   res_mgmt_once_##X

// ENTER: exp, run before enter block
// CHECK: exp, enter block if true
// EXIT : exp, run after exit block
//
// Use *break* to exit block.
#define WITH(ENTER, CHECK, EXIT)                                                \
    /* RES_MGMT_ONCE:           */                                              \
    /*  0: init                 */                                              \
    /* >0: create res succeed   */                                              \
    /* <0: create res failed    */                                              \
    for (int RES_MGMT_ONCE = 0; !RES_MGMT_ONCE;)                                \
                                                                                \
        /* create resource */                                                   \
        for (ENTER; !RES_MGMT_ONCE;                                             \
                /* destroy resource */                                          \
                RES_MGMT_ONCE > 0 && ((void)(EXIT), RES_MGMT_LEAKS_POP()))      \
                                                                                \
            /* capture break */                                                 \
            for (;!RES_MGMT_ONCE;)                                              \
                                                                                \
                /* run block */                                                 \
                if (RES_MGMT_EXP(CHECK) /* true if CHECK is empty */            \
                    ? (RES_MGMT_ONCE++, RES_MGMT_LEAKS_PUSH(), 1)               \
                    : (RES_MGMT_ONCE--, 0))


#define DEFER(...)  WITH(, , (__VA_ARGS__))


#ifndef RES_MGMT_NDEBUG
    // How many leaks or nested WITH() will be traced
#   ifndef RES_MGMT_LEAKS_MAX
#       define RES_MGMT_LEAKS_MAX (1u << 5u)
#   endif

    const char* res_mgmt_leaks[RES_MGMT_LEAKS_MAX+1];   // where (source line) leaks, i.e. jump out block directly
    unsigned    res_mgmt_leaks_cnt;                     // how many leaks

#   define RES_MGMT_LEAKS_INFO      ("[RES_MGMT " __FILE__ " " TO_STRING(__LINE__) "] leak!" )
#   define RES_MGMT_LEAKS_PUSH()                                                \
    (                                                                           \
        res_mgmt_leaks_cnt < RES_MGMT_LEAKS_MAX                                 \
            && (res_mgmt_leaks[res_mgmt_leaks_cnt++] = RES_MGMT_LEAKS_INFO)     \
    )
#   define RES_MGMT_LEAKS_POP()     (--res_mgmt_leaks_cnt)
#   define RES_MGMT_LEAKS()         (res_mgmt_leaks[res_mgmt_leaks_cnt] = 0, res_mgmt_leaks)

#   define TO_STRING(...)           TO_STRING_(__VA_ARGS__)
#   define TO_STRING_(...)          #__VA_ARGS__
#else
#   define RES_MGMT_LEAKS_PUSH(...) 0   // nop
#   define RES_MGMT_LEAKS_POP()     0   // nop
#   define RES_MGMT_LEAKS()         res_mgmt_leaks
#endif

// return true if E is an "empty" expression
#define RES_MGMT_EXP(E)                  RES_MGMT_EXP_01N(RES_MGMT_HAS_COMMA(E), RES_MGMT_HAS_COMMA(RES_MGMT_GET_COMMA E ()), E)
#define RES_MGMT_EXP_01N(...)            RES_MGMT_EXP_01N1(__VA_ARGS__)
#define RES_MGMT_EXP_01N1(D1,D2,E)       RES_MGMT_EXP_##D1##D2(E)
#define RES_MGMT_EXP_01(E)               1
#define RES_MGMT_EXP_00(E)               E
#define RES_MGMT_EXP_11(E)               E
#define RES_MGMT_HAS_COMMA(...)          RES_MGMT_HAS_COMMA1(__VA_ARGS__, RES_MGMT_HAS_COMMA_PADDING)
#define RES_MGMT_HAS_COMMA1(...)         RES_MGMT_HAS_COMMA2(__VA_ARGS__)
#define RES_MGMT_HAS_COMMA2(_1,_2,N,...) N
#define RES_MGMT_HAS_COMMA_PADDING       1,0,0
#define RES_MGMT_GET_COMMA(...)          ,

#endif // MOXITREL_RES_MGMT_H_
