#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(GENERIC_OPERATION, ISOLATED_OP) {
  arith::ConstantOp cst(makeConstAttribute(1));

  EXPECT_EQ(cst.getUsersNum(), 0);
  EXPECT_FALSE(cst.hasUsers());
  EXPECT_FALSE(cst.hasParentBasicBlock());
}

TEST(GENERIC_OPERATION, ADDING_AS_USER_TO_INPUTS) {
  arith::ConstantOp cst_1(makeConstAttribute(1));
  arith::ConstantOp cst_2(makeConstAttribute(1));

  arith::AddOp addOp1(&cst_1, &cst_2);

  EXPECT_TRUE(cst_1.getUsersNum() == 1);
  EXPECT_TRUE(cst_2.getUsersNum() == 1);

  const auto& cst1users = cst_1.getUsers();
  const auto& cst2users = cst_2.getUsers();

  User user1{&addOp1, 0};
  User user2{&addOp1, 1};

  EXPECT_EQ(cst1users.front(), user1);
  EXPECT_EQ(cst2users.front(), user2);

  arith::AddOp addOp2(&cst_2, &cst_1);

  EXPECT_TRUE(cst_1.getUsersNum() == 2);
  EXPECT_TRUE(cst_2.getUsersNum() == 2);
}

TEST(GENERIC_OPERATION, MOVING_OPERATION) {
  arith::ConstantOp cst_1(makeConstAttribute(1));
  arith::ConstantOp cst_2(makeConstAttribute(1));

  arith::AddOp addOp(&cst_1, &cst_2);

  builtin::CopyOp copyOp1(&addOp);
  builtin::CopyOp copyOp2(&addOp);

  EXPECT_EQ(addOp.getUsersNum(), 2);
  EXPECT_EQ(cst_1.getUsersNum(), 1);
  EXPECT_EQ(cst_2.getUsersNum(), 1);

  arith::AddOp newAddOp = std::move(addOp);

  EXPECT_EQ(addOp.getUsersNum(), 0);
  EXPECT_EQ(newAddOp.getUsersNum(), 2);

  EXPECT_EQ(cst_1.getUsersNum(), 2);
  EXPECT_EQ(cst_2.getUsersNum(), 2);
}

TEST(GENERIC_OPERATION, CLEARING_OPERATIONS_USES) {
  arith::ConstantOp cst_1(makeConstAttribute(1));
  arith::ConstantOp cst_2(makeConstAttribute(1));

  arith::AddOp addOp(&cst_1, &cst_2);

  builtin::CopyOp copyOp1(&addOp);
  builtin::CopyOp copyOp2(&addOp);

  addOp.clearAllUses();

  EXPECT_EQ(addOp.getUsersNum(), 0);
  EXPECT_TRUE(copyOp1.getInputAt(0).isEmpty());
  EXPECT_TRUE(copyOp2.getInputAt(0).isEmpty());
}

TEST(GENERIC_OPERATION, REPLACING_OPERATIONS_USES_1) {
  arith::ConstantOp cst_1(makeConstAttribute(1));
  arith::ConstantOp cst_2(makeConstAttribute(1));

  arith::AddOp addOp(&cst_1, &cst_2);

  builtin::CopyOp copyOp1(&addOp);
  builtin::CopyOp copyOp2(&addOp);

  arith::AddOp newAddOp(&cst_1, &cst_2);
  addOp.replaceAllUsesWith(newAddOp);

  EXPECT_EQ(addOp.getUsersNum(), 0);
  EXPECT_EQ(newAddOp.getUsersNum(), 2);

  EXPECT_FALSE(copyOp1.getInputAt(0).isEmpty());
  EXPECT_FALSE(copyOp2.getInputAt(0).isEmpty());

  EXPECT_EQ(copyOp1.getInputAt(0).getDefiningOp(), &newAddOp);
  EXPECT_EQ(copyOp2.getInputAt(0).getDefiningOp(), &newAddOp);
}

TEST(GENERIC_OPERATION, REPLACING_OPERATIONS_USES_2) {
  arith::ConstantOp cst_1(makeConstAttribute(1));
  arith::ConstantOp cst_2(makeConstAttribute(1));

  arith::AddOp addOp(&cst_1, &cst_2);

  builtin::CopyOp copyOp1(&addOp);
  builtin::CopyOp copyOp2(&addOp);

  arith::AddOp newAddOp(&cst_1, &cst_2);
  newAddOp.replaceAllUsesOf(addOp);

  EXPECT_EQ(addOp.getUsersNum(), 0);
  EXPECT_EQ(newAddOp.getUsersNum(), 2);

  EXPECT_FALSE(copyOp1.getInputAt(0).isEmpty());
  EXPECT_FALSE(copyOp2.getInputAt(0).isEmpty());

  EXPECT_EQ(copyOp1.getInputAt(0).getDefiningOp(), &newAddOp);
  EXPECT_EQ(copyOp2.getInputAt(0).getDefiningOp(), &newAddOp);
}

TEST(GENERIC_OPERATION, DISCONNECTING_OPERATION) {
  arith::ConstantOp cst_1(makeConstAttribute(1));
  arith::ConstantOp cst_2(makeConstAttribute(2));

  arith::AddOp addOp(&cst_1, &cst_2);
  builtin::CopyOp copyOp(&addOp);

  EXPECT_EQ(addOp.getUsersNum(), 1);
  EXPECT_EQ(cst_1.getUsersNum(), 1);
  EXPECT_EQ(cst_2.getUsersNum(), 1);
  EXPECT_FALSE(addOp.getInputAt(0).isEmpty());
  EXPECT_FALSE(addOp.getInputAt(1).isEmpty());
  EXPECT_FALSE(copyOp.getInputAt(0).isEmpty());

  addOp.disconnect();

  EXPECT_EQ(addOp.getUsersNum(), 0);
  EXPECT_EQ(cst_1.getUsersNum(), 0);
  EXPECT_EQ(cst_2.getUsersNum(), 0);
  EXPECT_TRUE(addOp.getInputAt(0).isEmpty());
  EXPECT_TRUE(addOp.getInputAt(1).isEmpty());
  EXPECT_TRUE(copyOp.getInputAt(0).isEmpty());
}

TEST(GENERIC_OPERATION, DELETING_OPERATION) {
  arith::ConstantOp cst_1(makeConstAttribute(1));
  arith::ConstantOp cst_2(makeConstAttribute(1));

  auto addOp = new arith::AddOp(&cst_1, &cst_2);

  builtin::CopyOp copyOp(addOp);

  EXPECT_EQ(addOp->getUsersNum(), 1);
  EXPECT_EQ(cst_1.getUsersNum(), 1);
  EXPECT_EQ(cst_2.getUsersNum(), 1);

  const auto& input = copyOp.getInput();
  EXPECT_FALSE(input.isEmpty());

  delete addOp;

  EXPECT_EQ(cst_1.getUsersNum(), 0);
  EXPECT_EQ(cst_2.getUsersNum(), 0);
  EXPECT_TRUE(input.isEmpty());
}

TEST(GENERIC_OPERATION, EXFAIL_EMPTY_INPUT) {
  arith::ConstantOp cst(makeConstAttribute(1));

  arith::AddOp add1(nullptr, &cst);
  arith::AddOp add2(&cst, nullptr);

  std::string msg;

  bool vres1 = add1.verify(msg);
  EXPECT_EQ(vres1, false);
  EXPECT_TRUE(msg.contains("input #0 is empty!"));

  bool vres2 = add2.verify(msg);
  EXPECT_EQ(vres2, false);
  EXPECT_TRUE(msg.contains("input #1 is empty!"));
}

TEST(GENERIC_OPERATION, MANUAL_INPUT_SETTING) {
  arith::AddOp addOp(nullptr, nullptr);

  arith::ConstantOp cst_1(makeConstAttribute(1));
  arith::ConstantOp cst_2(makeConstAttribute(2));

  EXPECT_TRUE(addOp.getInputAt(0).isEmpty());
  EXPECT_TRUE(addOp.getInputAt(1).isEmpty());
  EXPECT_EQ(cst_1.getUsersNum(), 0);
  EXPECT_EQ(cst_2.getUsersNum(), 0);

  addOp.setInput(0, &cst_1);
  addOp.setInput(1, &cst_2);

  EXPECT_FALSE(addOp.getInputAt(0).isEmpty());
  EXPECT_FALSE(addOp.getInputAt(1).isEmpty());
  EXPECT_EQ(cst_1.getUsersNum(), 1);
  EXPECT_EQ(cst_2.getUsersNum(), 1);

  arith::ConstantOp newCst(makeConstAttribute(3));

  addOp.setInput(0, &newCst);
  addOp.setInput(1, &newCst);

  EXPECT_FALSE(addOp.getInputAt(0).isEmpty());
  EXPECT_FALSE(addOp.getInputAt(1).isEmpty());
  EXPECT_EQ(cst_1.getUsersNum(), 0);
  EXPECT_EQ(cst_2.getUsersNum(), 0);
  EXPECT_EQ(newCst.getUsersNum(), 2);
}
