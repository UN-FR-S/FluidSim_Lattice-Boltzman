#include "ArrayLattice.cpp"
#include <gtest/gtest.h>

TEST(SimGridTest, setGet) {
  SimGrid simGrid = SimGrid(4, 4);
  std::vector<double> vec1(9, 1);
  simGrid.set_point(1, 1, vec1);

  double value1 = simGrid.get_point(0, 0, 0);
  double value2 = simGrid.get_point(1, 1, 0);

  ASSERT_FLOAT_EQ(value1, 0.0);
  ASSERT_FLOAT_EQ(value2, 1.0);

  ASSERT_FLOAT_EQ(simGrid.get_point(1, 1, 7), 1.0);
  ASSERT_FLOAT_EQ(simGrid.get_point(1, 1, 8), 1.0);
}

TEST(SimGridTest, setAll) {
  SimGrid simGrid = SimGrid(4, 4);
  simGrid.set_all_to_standard();
  for (int row = 0; row < simGrid.rows_; row++) {
    for (int col = 0; col < simGrid.cols_; col++) {
      ASSERT_FLOAT_EQ(simGrid.get_point(row, col, 0), 4.0 / 9.0);
    }
  }
  for (int func = 1; func < 5; func++) {
    for (int row = 0; row < simGrid.rows_; row++) {
      for (int col = 0; col < simGrid.cols_; col++) {
        ASSERT_FLOAT_EQ(simGrid.get_point(row, col, func), 1.0 / 9.0);
      }
    }
  }

  for (int func = 5; func < 9; func++) {
    for (int row = 0; row < simGrid.rows_; row++) {
      for (int col = 0; col < simGrid.cols_; col++) {
        ASSERT_FLOAT_EQ(simGrid.get_point(row, col, func), 1.0 / 36.0);
      }
    }
  }
}
TEST(SimGridTest, ShiftY) {
  SimGrid simGrid = SimGrid(4, 4);
  std::vector<double> vec1(9, 1);
  simGrid.set_point(1, 1, vec1);

  for (int func = 0; func < 9; func++) {

    simGrid.shift_Y(1, func);
    ASSERT_FLOAT_EQ(simGrid.get_point(1, 1, func), 0.0);
    ASSERT_FLOAT_EQ(simGrid.get_point(0, 1, func), 1.0);

    simGrid.shift_Y(-1, func);
    ASSERT_FLOAT_EQ(simGrid.get_point(1, 1, func), 1.0);
    ASSERT_FLOAT_EQ(simGrid.get_point(0, 1, func), 0.0);
  }
}

TEST(SimGridTest, ShiftX) {
  SimGrid simGrid = SimGrid(4, 4);
  std::vector<double> vec1(9, 1);
  simGrid.set_point(1, 1, vec1);

  for (int func = 0; func < 9; func++) {
    simGrid.shift_X(1, func);
    ASSERT_FLOAT_EQ(simGrid.get_point(1, 1, func), 0.0);
    ASSERT_FLOAT_EQ(simGrid.get_point(1, 2, func), 1.0);
    simGrid.shift_X(-1, func);
    ASSERT_FLOAT_EQ(simGrid.get_point(1, 2, func), 0.0);
    ASSERT_FLOAT_EQ(simGrid.get_point(1, 1, func), 1.0);
  }
}

TEST(SimGridTest, Shift) {
  SimGrid simGrid = SimGrid(4, 4);
  std::vector<double> vec1(9, 1);
  simGrid.set_point(1, 1, vec1);

  for (int func = 0; func < 9; func++) {
    simGrid.shift(1, 1, func);
    ASSERT_FLOAT_EQ(simGrid.get_point(1, 1, func), 0.0);
    ASSERT_FLOAT_EQ(simGrid.get_point(0, 2, func), 1.0);
    simGrid.shift(-1, -1, func);
    ASSERT_FLOAT_EQ(simGrid.get_point(0, 2, func), 0.0);
    ASSERT_FLOAT_EQ(simGrid.get_point(1, 1, func), 1.0);
  }
}

TEST(SimGridTest, Step) {
  SimGrid simGrid = SimGrid(4, 4);
  std::vector<double> vec1 = {4.0 / 9.0,  1.0 / 9.0,  1.0 / 9.0,
                              1.0 / 9.0,  1.0 / 9.0,  1.0 / 36.0,
                              1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0};
  simGrid.set_point(1, 1, vec1);

  simGrid.step();
  for (int func = 1; func < 9; func++) {

    ASSERT_FLOAT_EQ(simGrid.get_point(1, 1, func), 0.0);
    ASSERT_FLOAT_EQ(simGrid.get_point(1 - simGrid.directionVector_[func].second,
                                      1 + simGrid.directionVector_[func].first,
                                      func),
                    vec1[func]);
  }
}

TEST(SimGridTest, collision) {
  double e = 0.05;
  SimGrid simGrid = SimGrid(4, 4);

  std::vector<double> vec1 = {4.0 / 9.0,     1.0 / 9.0 + e, 1.0 / 9.0,
                              1.0 / 9.0 - e, 1.0 / 9.0,     1.0 / 36.0,
                              1.0 / 36.0,    1.0 / 36.0,    1.0 / 36.0};
  simGrid.set_point(1, 1, vec1);
  simGrid.collision();

  std::vector<double> vec2{
      0.4377777777777777,   0.14777777777777776,  0.10944444444444443,
      0.08111111111111109,  0.10944444444444443,  0.03694444444444444,
      0.020277777777777773, 0.020277777777777773, 0.03694444444444444};
  for (int f = 0; f < 9; f++) {
    ASSERT_FLOAT_EQ(simGrid.get_point(1, 1, f), vec2[f]);
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
