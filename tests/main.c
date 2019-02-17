#include <stdio.h>
#include <stdbool.h>

bool run_all_tests(void);

int main(int argc, const char * argv[]) {
    printf("Run Basic Interpreter Tests\n");

    bool has_failures = run_all_tests();

    return has_failures ? 1 : 0;
}
