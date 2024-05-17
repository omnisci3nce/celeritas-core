#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER(Pool) {
  // TODO: test cases
  RUN_TEST_CASE(Pool, Initialisation);
  RUN_TEST_CASE(Pool, TypedPool);
}

static void RunAllTests(void) { RUN_TEST_GROUP(Pool); }

int main(int argc, const char* argv[]) { return UnityMain(argc, argv, RunAllTests); }
