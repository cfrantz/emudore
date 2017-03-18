#ifndef EMUDORE_SRC_PBMACRO_H
#define EMUDORE_SRC_PBMACRO_H

#define PP_NARG(...)    PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...)   PP_ARG_N(__VA_ARGS__)

#define PP_ARG_N( \
        _1, _2, _3, _4, _5, _6, _7, _8, _9,_10,  \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
        _61,_62,_63,N,...) N

#define PP_RSEQ_N() \
        63,62,61,60,                   \
        59,58,57,56,55,54,53,52,51,50, \
        49,48,47,46,45,44,43,42,41,40, \
        39,38,37,36,35,34,33,32,31,30, \
        29,28,27,26,25,24,23,22,21,20, \
        19,18,17,16,15,14,13,12,11,10, \
        9,8,7,6,5,4,3,2,1,0

#define PASTE_(x, y) x ## y
#define PASTE(x, y) PASTE_(x, y)

#define APPLYX1(X, a0) \
    X(a0)
#define APPLYX2(X, a0, a1) \
    X(a0) X(a1)
#define APPLYX3(X, a0, a1, a2) \
    X(a0) X(a1) X(a2)
#define APPLYX4(X, a0, a1, a2, a3) \
    X(a0) X(a1) X(a2) X(a3)
#define APPLYX5(X, a0, a1, a2, a3, a4) \
    X(a0) X(a1) X(a2) X(a3) X(a4)
#define APPLYX6(X, a0, a1, a2, a3, a4, a5) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5)
#define APPLYX7(X, a0, a1, a2, a3, a4, a5, a6) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6)
#define APPLYX8(X, a0, a1, a2, a3, a4, a5, a6, a7) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7)

#define APPLYX9(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                b0) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0)
#define APPLYX10(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1)
#define APPLYX11(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2)
#define APPLYX12(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3)
#define APPLYX13(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4)
#define APPLYX14(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5)
#define APPLYX15(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5, b6) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5) X(b6)
#define APPLYX16(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5, b6, b7) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5) X(b6) X(b7)

#define APPLYX17(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5, b6, b7, \
                 c0) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5) X(b6) X(b7) \
    X(b0)
#define APPLYX18(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5, b6, b7, \
                 c0, c1) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5) X(b6) X(b7) \
    X(c0) X(c1)
#define APPLYX19(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5, b6, b7, \
                 c0, c1, c2) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5) X(b6) X(b7) \
    X(c0) X(c1) X(c2)
#define APPLYX20(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5, b6, b7, \
                 c0, c1, c2, c3) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5) X(b6) X(b7) \
    X(c0) X(c1) X(c2) X(c3)
#define APPLYX21(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5, b6, b7, \
                 c0, c1, c2, c3, c4) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5) X(b6) X(b7) \
    X(c0) X(c1) X(c2) X(c3) X(c4)
#define APPLYX22(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5, b6, b7, \
                 c0, c1, c2, c3, c4, c5) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5) X(b6) X(b7) \
    X(c0) X(c1) X(c2) X(c3) X(c4) X(c5)
#define APPLYX23(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5, b6, b7, \
                 c0, c1, c2, c3, c4, c5, c6) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5) X(b6) X(b7) \
    X(c0) X(c1) X(c2) X(c3) X(c4) X(c5) X(c6)
#define APPLYX24(X, a0, a1, a2, a3, a4, a5, a6, a7, \
                 b0, b1, b2, b3, b4, b5, b6, b7, \
                 c0, c1, c2, c3, c4, c5, c6, c7) \
    X(a0) X(a1) X(a2) X(a3) X(a4) X(a5) X(a6) X(a7) \
    X(b0) X(b1) X(b2) X(b3) X(b4) X(b5) X(b6) X(b7) \
    X(c0) X(c1) X(c2) X(c3) X(c4) X(c5) X(c6) X(c7)
#define APPLYX_(M, ...) M(__VA_ARGS__)
#define APPLYX(X, ...) APPLYX_(PASTE(APPLYX, PP_NARG(__VA_ARGS__)), X, __VA_ARGS__)


#define SAVE_FIELD(pbfield, name) state->set_##pbfield(name);
#define LOAD_FIELD(pbfield, name) name = state->pbfield();

#define SAVE_FIELD1(x) state->set_##x(x##_);
#define LOAD_FIELD1(x) x##_ = state->x();

#define LOAD(...) APPLYX(LOAD_FIELD1, __VA_ARGS__)
#define SAVE(...) APPLYX(SAVE_FIELD1, __VA_ARGS__)

#endif // EMUDORE_SRC_PBMACRO_H
