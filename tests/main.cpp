//
// luna
//
// Copyright © 2016, D.E. Goodman-Wilson
//

#include <gtest/gtest.h>

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

    return RUN_ALL_TESTS();
}
