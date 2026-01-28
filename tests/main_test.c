#include "main_test.h"

#include <check.h>
#include <stdlib.h>

int main(void) {
  Suite *list_cases[] = {all_tests_suite(), NULL};
  int number_failed = 0;

  for (int i = 0; list_cases[i]; i++) {
    SRunner *runner = srunner_create(list_cases[i]);
    srunner_set_fork_status(runner, CK_NOFORK);
    srunner_run_all(runner, CK_NORMAL);
    number_failed += srunner_ntests_failed(runner);
    srunner_free(runner);
  }
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
