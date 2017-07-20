//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//

#include <gtest/gtest.h>

#include <execinfo.h>

void handler(int sig)
{
    void *array[20];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 20);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(EXIT_FAILURE);
}

class Environment :
        public ::testing::Environment
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new Environment());

    signal(SIGSEGV, handler);

    return RUN_ALL_TESTS();
}
