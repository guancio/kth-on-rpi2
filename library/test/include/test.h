
#ifndef _TEST_H_
#define _TEST_H_

typedef struct  {
    int errors_total, errors_group;
    int tests_total, tests_group;
    int count_group;
    char *group_name;
} test_context_t;




/* these functions are implemented by the framework */
#define abs_(a)  ((a) < 0 ? -(a) : a)

#define test__(ctx, msg, succ, format, a, b) \
 do { test_update(ctx, msg, succ, __FILE__, __LINE__, format, a, b); } while(0)

#define test_fail(ctx, msg) test__(ctx, msg, 0, "(FORCED FAIL)", 0, 0)

#define test_equal(ctx, msg, i1, i2) test__(ctx, msg, (i1) == (i2), "%d == %d", (i1), (i2))
#define test_not_equal(ctx, msg, i1, i2) test__(ctx, msg, (i1) != (i2), "%d != %d", (i1), (i2))
#define test_equal_with_delta(ctx, msg, i1, i2, delta) test__(ctx, msg, abs((i1) - (i2)) < delta, "%f == %f", (i1), (i2))

#define test_less_than(ctx, msg, i1, i2) test__(ctx, msg, (i1) < (i2), "%d < %d", (i1), (i2))
#define test_less_or_equal(ctx, msg, i1, i2) test__(ctx, msg, (i1) <= (i2), "%d <= %d", (i1), (i2))

#define test_larger_than(ctx, msg, i1, i2) test__(ctx, msg, (i1) > (i2), "%d > %d", (i1), (i2))
#define test_larger_or_equal(ctx, msg, i1, i2) test__(ctx, msg, (i1) >= (i2), "%d >= %d", (i1), (i2))


#define test_not_null(ctx, msg, ptr) test__(ctx, msg, (ptr) != 0, "pointer = %08lx", (uint32_t) (ptr), 0)
#define test_null(ctx, msg, ptr) test__(ctx, msg, (ptr) == 0, "pointer = %08lx", (uint32_t) (ptr), 0)

//extern void test_float_equal(test_context_t *ctx, char *msg, float i1, float i2, float delta);


extern void test_init(test_context_t * ctx);
extern void test_cleanup(test_context_t * ctx);

extern void test_mark(test_context_t * ctx, char *msg);

extern void test_group_start(test_context_t * ctx, char *name);
extern void test_group_end(test_context_t * ctx);


extern void test_update(test_context_t *ctx, const char *msg, int succ, 
                 const char *filename, int line, 
                 const char *format, ...);

#endif
