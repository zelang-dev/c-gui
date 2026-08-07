typedef union {
    int		    ival;
    XrmQuark	    qval;
    BoxPtr	    bval;
    BoxParamsPtr    pval;
    GlueRec	    gval;
    LayoutDirection lval;
    ExprPtr	    eval;
    Operator	    oval;
} YYSTYPE;
#define	OC	258
#define	CC	259
#define	OA	260
#define	CA	261
#define	OP	262
#define	CP	263
#define	NAME	264
#define	NUMBER	265
#define	INFINITY	266
#define	VERTICAL	267
#define	HORIZONTAL	268
#define	EQUAL	269
#define	DOLLAR	270
#define	PLUS	271
#define	MINUS	272
#define	TIMES	273
#define	DIVIDE	274
#define	PERCENTOF	275
#define	PERCENT	276
#define	WIDTH	277
#define	HEIGHT	278
#define	UMINUS	279
#define	UPLUS	280


extern YYSTYPE LayYYlval;
