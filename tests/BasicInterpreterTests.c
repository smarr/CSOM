#include <stdio.h>

#include <compiler/Parser.h>
#include <memory/gc.h>

#include <vm/Universe.h>
#include <vmobjects/VMClass.h>
#include <vmobjects/VMDouble.h>
#include <vmobjects/VMInteger.h>

#define fprintf_pass(f,x) \
  va_list ap; \
  va_start(ap, (x)); \
  (void)vfprintf((f), (x), ap); \
  va_end(ap)


typedef enum {
    INTEGER, CLASS, SYMBOL, DOUBLE
} ResultType;

typedef struct {
    const char* const class_name;
    const char* const method_name;
    void * const expected_result;
    ResultType expected_type;
} Test;

static const double dbl375 = 3.75;

static const Test tests[] = {
//    {"Self", "assignSuper", 42, ProgramDefinitionError.class},
//    {"Self", "assignSelf", 42, ProgramDefinitionError.class},

    {"MethodCall", "test", (void*) 42, INTEGER},
    {"MethodCall", "test2", (void*) 42, INTEGER},

    {"NonLocalReturn", "test1", (void*) 42, INTEGER},
    {"NonLocalReturn", "test2", (void*) 43, INTEGER},
    {"NonLocalReturn", "test3", (void*)  3, INTEGER},
    {"NonLocalReturn", "test4", (void*) 42, INTEGER},
    {"NonLocalReturn", "test5", (void*) 22, INTEGER},

    {"Blocks", "testArg1", (void*) 42, INTEGER},
    {"Blocks", "testArg2", (void*) 77, INTEGER},
    {"Blocks", "testArgAndLocal",   (void*) 8, INTEGER},
    {"Blocks", "testArgAndContext", (void*) 8, INTEGER},
    {"Blocks", "testEmptyZeroArg", 1, INTEGER},
    {"Blocks", "testEmptyOneArg", 1, INTEGER},
    {"Blocks", "testEmptyTwoArg", 1, INTEGER},

    {"Return", "testReturnSelf", "Return", CLASS},
    {"Return", "testReturnSelfImplicitly", "Return", CLASS},
    {"Return", "testNoReturnReturnsSelf", "Return", CLASS},
    {"Return", "testBlockReturnsImplicitlyLastValue", (void*) 4, INTEGER},

    {"IfTrueIfFalse", "test",  (void*) 42, INTEGER},
    {"IfTrueIfFalse", "test2", (void*) 33, INTEGER},
    {"IfTrueIfFalse", "test3", (void*)  4, INTEGER},

    {"CompilerSimplification", "testReturnConstantSymbol", "constant", SYMBOL},
    {"CompilerSimplification", "testReturnConstantInt", (void*) 42, INTEGER},
    {"CompilerSimplification", "testReturnSelf", "CompilerSimplification", CLASS},
    {"CompilerSimplification", "testReturnSelfImplicitly", "CompilerSimplification",
        CLASS},
    {"CompilerSimplification", "testReturnArgumentN", (void*) 55, INTEGER},
    {"CompilerSimplification", "testReturnArgumentA", (void*) 44, INTEGER},
    {"CompilerSimplification", "testSetField", "foo", SYMBOL},
    {"CompilerSimplification", "testGetField", (void*) 40, INTEGER},

    {"Hash", "testHash", (void*) 444, INTEGER},

    {"Arrays", "testEmptyToInts", (void*) 3, INTEGER},
    {"Arrays", "testPutAllInt", (void*) 5, INTEGER},
    {"Arrays", "testPutAllNil", "Nil", CLASS},
    {"Arrays", "testPutAllBlock", (void*) 3, INTEGER},
    {"Arrays", "testNewWithAll", (void*) 1, INTEGER},

    {"BlockInlining", "testNoInlining", (void*) 1, INTEGER},
    {"BlockInlining", "testOneLevelInlining", (void*) 1, INTEGER},
    {"BlockInlining", "testOneLevelInliningWithLocalShadowTrue", (void*) 2, INTEGER},
    {"BlockInlining", "testOneLevelInliningWithLocalShadowFalse", (void*) 1, INTEGER},

    {"BlockInlining", "testBlockNestedInIfTrue", (void*) 2, INTEGER},
    {"BlockInlining", "testBlockNestedInIfFalse", (void*) 42, INTEGER},

    {"BlockInlining", "testDeepNestedInlinedIfTrue", (void*) 3, INTEGER},
    {"BlockInlining", "testDeepNestedInlinedIfFalse", (void*) 42, INTEGER},

    {"BlockInlining", "testDeepNestedBlocksInInlinedIfTrue", (void*) 5, INTEGER},
    {"BlockInlining", "testDeepNestedBlocksInInlinedIfFalse", (void*) 43, INTEGER},

    {"BlockInlining", "testDeepDeepNestedTrue", (void*) 9, INTEGER},
    {"BlockInlining", "testDeepDeepNestedFalse", (void*) 43, INTEGER},

    {"BlockInlining", "testToDoNestDoNestIfTrue", (void*) 2, INTEGER},

    {"NonLocalVars", "testWriteDifferentTypes", (void*) &dbl375, DOUBLE},

    {"ObjectCreation", "test", (void*) 1000000, INTEGER},

    {"Regressions", "testSymbolEquality", (void*) 1, INTEGER},
    {"Regressions", "testSymbolReferenceEquality", (void*) 1, INTEGER},
    {"Regressions", "testUninitializedLocal", 1, INTEGER},
    {"Regressions", "testUninitializedLocalInBlock", 1, INTEGER},

    {"BinaryOperation", "test", (void*) 11, INTEGER},

    {"NumberOfTests", "numberOfTests", (void*) 57, INTEGER},

    {NULL}
};


static bool assertion_failed = false;

static void assert_desc(bool condition, const char* fmt, ...) {
    if(!condition) {
        fprintf_pass(stdout, fmt);
        assertion_failed = true;
    }
}

static void assert_equals(pVMObject result, Test test) {
    if (test.expected_type == INTEGER) {
        int64_t expected = (int64_t) test.expected_result;
        pVMInteger actual = (pVMInteger) result;

        assert_desc(expected == SEND(actual, get_embedded_integer),
                    "Assertion failed. Expected %lld, but got %lld.",
                    expected, SEND(actual, get_embedded_integer));
        return;
    }

    if (test.expected_type == DOUBLE) {
        double expected = *((double*) test.expected_result);
        pVMDouble actual = (pVMDouble) result;

        assert_desc(SEND(actual, get_embedded_double) - expected < 1e-15,
                    "Assertion failed. Expected %f, but got %f.",
                    expected, SEND(actual, get_embedded_double));
        return;
    }

    if (test.expected_type == CLASS) {
        const char* expected = (const char*) test.expected_result;
        pVMClass actual = (pVMClass) result;
        const char* actual_class_name = SEND(SEND(actual, get_name), get_rawChars);

        assert_desc(strcmp(expected, actual_class_name) == 0,
                    "Assertion failed. Expected class %s, but got %s",
                    expected, actual_class_name);
        return;
    }

    if (test.expected_type == SYMBOL) {
        const char* expected = (const char*) test.expected_result;
        pVMSymbol actual = (pVMSymbol) result;
        const char* actual_str = SEND(actual, get_rawChars);

        assert_desc(strcmp(expected, actual_str) == 0,
                    "Assertion failed. Expected symbol %s, but got %s",
                    expected, actual_str);
        return;
    }
}

void run_test(Test test) {
    Universe_set_classpath("Smalltalk:TestSuite/BasicInterpreterTests");
    pVMObject result = Universe_interpret(test.class_name, test.method_name);

    assert_equals(result, test);

    gc_finalize();
}


bool run_all_tests() {
    Parser_init_constants();
    
    bool has_failures = false;
    for (int i = 0; tests[i].class_name != NULL; i += 1) {
        printf("Test: %s>>#%s\n", tests[i].class_name, tests[i].method_name);

        run_test(tests[i]);

        if (assertion_failed) {
            assertion_failed = false;
            has_failures = true;
        } else {
            printf("\tsucceeded\n");
        }
    }

    return has_failures;
}
