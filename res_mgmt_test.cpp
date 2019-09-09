#include "res_mgmt.h"
#include "gtest/gtest.h"

TEST(WITH, CreateFailed)
{
    res_mgmt_leaks_cnt = 0;

    bool doInit = false;
    bool doExit = false;
    bool sk = false;
    bool fk = false;

    WITH(doInit = true, false, doExit = true) {
        sk = true;
    } else {
        fk = true;
    }

    // run fk block
    EXPECT_TRUE (doInit);
    EXPECT_FALSE(doExit);
    EXPECT_FALSE(sk);
    EXPECT_TRUE (fk);
    // no leak
    EXPECT_EQ(RES_MGMT_CHECK(), 0);
}

TEST(WITH, CreateSucceed)
{
    res_mgmt_leaks_cnt = 0;

    bool doInit = false;
    bool doExit = false;
    bool sk = false;
    bool fk = false;

    WITH(doInit = true, true, doExit = true) {
        sk = true;
    } else {
        fk = true;
    }

    // run sk block
    EXPECT_TRUE (doInit);
    EXPECT_TRUE (doExit);
    EXPECT_TRUE (sk);
    EXPECT_FALSE(fk);
    // no leak
    EXPECT_EQ(RES_MGMT_CHECK(), 0);
}

TEST(WITH, Nested)
{
    res_mgmt_leaks_cnt = 0;

    bool doInit1 = false;
    bool doExit1 = false;
    bool sk1 = false;
    bool fk1 = false;
    bool doInit2 = false;
    bool doExit2 = false;
    bool sk2 = false;
    bool fk2 = false;

    WITH(doInit1 = true, true, doExit1 = true) {
        WITH(doInit2 = true, true, doExit2 = true) {
            sk1 = true;
            sk2 = true;
        } else {
            fk2 = true;
        }
    } else {
        fk1 = true;
    }

    EXPECT_TRUE (doInit1);
    EXPECT_TRUE (doExit1);
    EXPECT_TRUE (doInit2);
    EXPECT_TRUE (doExit2);
    EXPECT_TRUE (sk1);
    EXPECT_TRUE (sk2);
    EXPECT_FALSE(fk2);
    EXPECT_FALSE(fk1);
    // no leak
    EXPECT_EQ(RES_MGMT_CHECK(), 0);
}


TEST(WITH, JumpOutBlockLeak)
{
    res_mgmt_leaks_cnt = 0;

    bool doInit = false;
    bool doExit = false;
    bool sk = false;
    bool fk = false;
    bool afterJump = false;


    WITH(doInit = true, true, doExit = true) {
        sk = true;
        goto jumpOutBlock;
        afterJump = true;
    } else {
        fk = true;
    }
jumpOutBlock:

    EXPECT_TRUE (doInit);
    EXPECT_FALSE(doExit);
    EXPECT_TRUE (sk);
    EXPECT_FALSE(afterJump);
    EXPECT_FALSE(fk);
    // 1 leak
    EXPECT_EQ(RES_MGMT_CHECK(), 1);
}

TEST(WITH, BreakNoLeak)
{
    res_mgmt_leaks_cnt = 0;

    bool doInit = false;
    bool doExit = false;
    bool sk = false;
    bool fk = false;
    bool afterBreak = false;

    WITH(doInit = true, true, doExit = true) {
        sk = true;
        break;
        afterBreak = true;
    } else {
        fk = true;
    }

    EXPECT_TRUE (doInit);
    EXPECT_TRUE (doExit);
    EXPECT_TRUE (sk);
    EXPECT_FALSE(afterBreak);
    EXPECT_FALSE(fk);
    // no leak
    EXPECT_EQ(RES_MGMT_CHECK(), 0);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
