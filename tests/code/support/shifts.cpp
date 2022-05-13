#include <tuple>

#include "support/rightshifts.hpp"
#include "host/types.hpp"
#include "gtest/gtest.h"

class LShr8  :public ::testing::TestWithParam<std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>> {  
};

TEST_P(LShr8, KK) {
  MiniMC::BV8 left = std::get<0> (GetParam());
  MiniMC::BV8 right = std::get<1> (GetParam());
  MiniMC::BV8 expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::lshr (left,right),expected);
}

INSTANTIATE_TEST_CASE_P(
			LogicalShift8BitNumbers,
			LShr8,
			::testing::Values(
					  std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>(2,1,1)
					  )
			);



class LShr16  :public ::testing::TestWithParam<std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>> {  
};

TEST_P(LShr16, KK) {
  MiniMC::BV16 left = std::get<0> (GetParam());
  MiniMC::BV16 right = std::get<1> (GetParam());
  MiniMC::BV16 expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::lshr (left,right),expected);
}

INSTANTIATE_TEST_CASE_P(
			LogicalShift16BitNumbers,
			LShr16,
			::testing::Values(
					  std::tuple<MiniMC::BV16,MiniMC::BV16,MiniMC::BV16>(2,1,1)
					  )
			);


class LShr32  :public ::testing::TestWithParam<std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>> {  
};

TEST_P(LShr32, KK) {
  MiniMC::BV8 left = std::get<0> (GetParam());
  MiniMC::BV8 right = std::get<1> (GetParam());
  MiniMC::BV8 expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::lshr (left,right),expected);
}

INSTANTIATE_TEST_CASE_P(
			LogicalShift32BitNumbers,
			LShr32,
			::testing::Values(
					  std::tuple<MiniMC::uint32_t,MiniMC::uint32_t,MiniMC::uint32_t>(2,1,1)
					  )
			);


class LShr64  :public ::testing::TestWithParam<std::tuple<MiniMC::BV8,MiniMC::BV8,MiniMC::BV8>> {  
};

TEST_P(LShr64, KK) {
  MiniMC::BV8 left = std::get<0> (GetParam());
  MiniMC::BV8 right = std::get<1> (GetParam());
  MiniMC::BV8 expected = std::get<2> (GetParam());
  
  EXPECT_EQ(MiniMC::Support::lshr (left,right),expected);
}

INSTANTIATE_TEST_CASE_P(
			LogicalShift64BitNumbers,
			LShr64,
			::testing::Values(
					  std::tuple<MiniMC::uint64_t,MiniMC::uint64_t,MiniMC::uint64_t>(2,1,1)
					  )
			);





class AShr8  :public ::testing::TestWithParam<std::tuple<MiniMC::int8_t,MiniMC::int8_t,MiniMC::int8_t>> {  
};

TEST_P(AShr8, KK) {
  MiniMC::BV8 left = MiniMC::bit_cast<MiniMC::int8_t, MiniMC::BV8>(std::get<0> (GetParam()));
  MiniMC::BV8 right = MiniMC::bit_cast<MiniMC::int8_t, MiniMC::BV8>(std::get<1> (GetParam()));
  MiniMC::int8_t expected = std::get<2> (GetParam());
  auto res = MiniMC::bit_cast<MiniMC::BV8, MiniMC::int8_t>(MiniMC::Support::ashr (left,right)); 
  EXPECT_EQ(res,expected);
}

INSTANTIATE_TEST_CASE_P(
			ArithmeticShift8BitNumbers,
			AShr8,
			::testing::Values(
					  std::tuple<MiniMC::int8_t,MiniMC::int8_t,MiniMC::int8_t>(-2,1,-1)
					  )
			);


class AShr16  :public ::testing::TestWithParam<std::tuple<MiniMC::int16_t,MiniMC::int16_t,MiniMC::int16_t>> {  
};

TEST_P(AShr16, KK) {
  MiniMC::BV16 left = MiniMC::bit_cast<MiniMC::int16_t, MiniMC::uint16_t>(std::get<0> (GetParam()));
  MiniMC::BV16 right = MiniMC::bit_cast<MiniMC::int16_t, MiniMC::uint16_t>(std::get<1> (GetParam()));
  MiniMC::int16_t expected = std::get<2> (GetParam());
  auto res = MiniMC::bit_cast<MiniMC::uint16_t, MiniMC::int16_t>(MiniMC::Support::ashr (left,right)); 
  EXPECT_EQ(res,expected);
}

INSTANTIATE_TEST_CASE_P(
			ArithmeticShift16BitNumbers,
			AShr16,
			::testing::Values(
					  std::tuple<MiniMC::int16_t,MiniMC::int16_t,MiniMC::int16_t>(-2,1,-1)
					  )
			);




class AShr32  :public ::testing::TestWithParam<std::tuple<MiniMC::int32_t,MiniMC::int32_t,MiniMC::int32_t>> {  
};

TEST_P(AShr32, KK) {
  MiniMC::BV32 left = MiniMC::bit_cast<MiniMC::int32_t, MiniMC::uint32_t>(std::get<0> (GetParam()));
  MiniMC::BV32 right = MiniMC::bit_cast<MiniMC::int32_t, MiniMC::uint32_t>(std::get<1> (GetParam()));
  MiniMC::int32_t expected = std::get<2> (GetParam());
  auto res = MiniMC::bit_cast<MiniMC::uint32_t, MiniMC::int32_t>(MiniMC::Support::ashr (left,right)); 
  EXPECT_EQ(res,expected);
}

INSTANTIATE_TEST_CASE_P(
			ArithmeticShift32BitNumbers,
			AShr32,
			::testing::Values(
					  std::tuple<MiniMC::int32_t,MiniMC::int32_t,MiniMC::int32_t>(-2,1,-1)
					  )
			);



class AShr64  :public ::testing::TestWithParam<std::tuple<MiniMC::int64_t,MiniMC::int64_t,MiniMC::int64_t>> {  
};

TEST_P(AShr64, KK) {
  MiniMC::uint64_tBV64 left = MiniMC::bit_cast<MiniMC::int64_t, MiniMC::uint64_t>(std::get<0> (GetParam()));
  MiniMC::uint64_tBV64 right = MiniMC::bit_cast<MiniMC::int64_t, MiniMC::uint64_t>(std::get<1> (GetParam()));
  MiniMC::int64_t expected = std::get<2> (GetParam());
  auto res = MiniMC::bit_cast<MiniMC::uint64_t, MiniMC::int64_t>(MiniMC::Support::ashr (left,right)); 
  EXPECT_EQ(res,expected);
}

INSTANTIATE_TEST_CASE_P(
			ArithmeticShift64BitNumbers,
			AShr64,
			::testing::Values(
					  std::tuple<MiniMC::int64_t,MiniMC::int64_t,MiniMC::int64_t>(-2,1,-1)
					  )
			);



