
#include <SFML/Graphics.hpp>
#include <algorithm> // std::rotate
#include <iomanip>   // Print_grid uses set_w
#include <iostream>
#include <thread>
#include <vector>

using datatype = double;

class SimGrid {
public:
  // 9D Vector containing 2D fields. In these the
  std::vector<std::vector<std::vector<datatype>>> grid_;
  std::vector<std::vector<std::vector<datatype>>> grid_T_;
  std::vector<std::vector<std::vector<datatype>>> grid_C02_;
  std::vector<std::pair<int, int>> directionVector_;
  std::vector<std::pair<int, int>> fanPositions_;
  std::vector<datatype> weights_;
  std::vector<std::vector<datatype>> density_;
  std::vector<std::vector<datatype>> T_density;
  std::vector<std::vector<datatype>> C02_density;
  std::pair<datatype, datatype> u_;
  size_t rows_;
  size_t cols_;
  // Vector mit Threads

  // Delta t, equivalent to amount of iteration steps made.
  int dt = 0;

public:
  // Konstruktor, r = rows, c = cols
  SimGrid(size_t r, size_t c)
      : rows_(r), cols_(c), grid_(9, std::vector<std::vector<datatype>>(
                                         r, std::vector<datatype>(c, 0.0))),
        grid_T_(9, std::vector<std::vector<datatype>>(
                       r, std::vector<datatype>(c, 0.0))),
        grid_C02_(9, std::vector<std::vector<datatype>>(
                         r, std::vector<datatype>(c, 0.0))),

        directionVector_{{0, 0}, {1, 0},  {0, 1},   {-1, 0}, {0, -1},
                         {1, 1}, {-1, 1}, {-1, -1}, {1, -1}},

        fanPositions_{{10, 10}, {10, 11}, {10, 12}, {10, 13}, {10, 14},
                      {10, 15}, {10, 16}, {10, 17}, {10, 18}, {10, 19}},
        density_(r, std::vector<datatype>(c, 0.0)),
        weights_{4.0 / 9.0,  1.0 / 9.0,  1.0 / 9.0,  1.0 / 9.0, 1.0 / 9.0,
                 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0} {}

  // Returns Value at certain Point y,x, function
  datatype get_point(size_t row, size_t col, int functionIndex) {
    return grid_.at(functionIndex).at(row).at(col);
  }
  // Sets value of 9 functionvalues at a chosen point of the grid. Input Vector
  // has to be 9D otherwise -1 is returned.
  int set_point(size_t row, size_t col, std::vector<datatype> newValues) {
    if (newValues.size() != 9) {
      return -1;
    }
    for (int i = 0; i < 9; i++) {
      grid_.at(i).at(row).at(col) = newValues[i];
    }
    return 0;
  }

  // Sets all Points to the common dis of 4.0 / 9.0, 1/9, 1/36

  void set_all_to_standard() {
    datatype val = 4.0 / 9.0;
    for (int func = 0; func < 9; func++) {
      if (func > 0 and func <= 4) {
        val = 1.0 / 9.0;
      } else if (func > 4) {
        val = 1.0 / 36.0;
      }

      for (int row = 0; row < rows_; row++) {
        for (int col = 0; col < cols_; col++) {
          grid_.at(func).at(row).at(col) = val;
          grid_T_.at(func).at(row).at(col) = val;
          grid_C02_.at(func).at(row).at(col) = val;
        }
      }
    }
  }

  void set_half_to_O1(std::vector<std::vector<std::vector<datatype>>> &grid) {
    datatype val = 4.0 / 9.0;
    for (int func = 0; func < 9; func++) {
      if (func > 0 and func <= 4) {
        val = 1.0 / 9.0;
      } else if (func > 4) {
        val = 1.0 / 36.0;
      }
      val *= 0.1;
      for (int row = 0; row < (rows_ / 2); row++) {
        for (int col = 0; col < cols_; col++) {

          grid.at(func).at(row).at(col) = val;
        }
      }
    }
  }
  // Shifts all rows down or up based on the index. Positive is up.
  void shift_Y(int shift, size_t function,
               std::vector<std::vector<std::vector<datatype>>> &grid) {
    if (shift == 0) {
      return;
    }
    if (shift < 0) {
      std::rotate(grid[function].begin(), grid[function].end() + shift,
                  grid[function].end());
    } else {
      std::rotate(grid[function].begin(), grid[function].begin() + shift,
                  grid[function].end());
    }
  }

  // Shifts all Cols to the right or left by the amount of the Index. Positive
  // is
  void shift_X(int shift, size_t function,
               std::vector<std::vector<std::vector<datatype>>> &grid) {
    if (shift == 0) {
      return;
    }
    if (shift < 0) {
      for (int row = 0; row < rows_; row++) {
        std::rotate(grid[function][row].begin(),
                    grid[function][row].begin() - shift,
                    grid[function][row].end());
      }
    } else {
      for (int row = 0; row < rows_; row++) {
        std::rotate(grid[function][row].begin(),
                    grid[function][row].end() - shift,
                    grid[function][row].end());
      }
    }
  }

  // Combined Shift
  void shift(int x_shift, int y_shift, size_t function) {
    shift_X(x_shift, function, grid_);
    shift_Y(y_shift, function, grid_);
    shift_X(x_shift, function, grid_T_);
    shift_Y(y_shift, function, grid_T_);
    shift_X(x_shift, function, grid_C02_);
    shift_Y(y_shift, function, grid_C02_);
  }

  // Finishes Stream Stage.
  void step() {
    for (int f = 0; f < 9; f++) {
      shift(directionVector_[f].first, directionVector_[f].second, f);
    }
    dt++;
  }

  //
  void collision(datatype omega = 1, datatype fan_speed = 1) {
    // density is a 2D grid of size of the grid_
    // Add all values of f(x) to
    density_ = grid_[0];
    for (int f = 1; f < 9; f++) {
      for (int row = 0; row < rows_; row++) {
        for (int col = 0; col < cols_; col++) {
          density_[row][col] += grid_[f][row][col];
        }
      }
    }
    // Compute f* if density is not 0
    datatype f_i;
    datatype u_x;
    datatype u_y;
    // d_u is u_ times the direction vector
    datatype d_u;
    for (int row = 0; row < rows_; row++) {
      for (int col = 0; col < cols_; col++) {
        if (density_[row][col] == 0) {
          continue;
        }

        // If it is Ventilator
        if (is_fan_proportional(row, col)) {
          u_.first = 0;
          u_.second = fan_speed;
        } else {
          u_x = 0;
          u_y = 0;
          for (int f = 0; f < 9; f++) {
            f_i = grid_[f][row][col];
            u_x += directionVector_[f].first * f_i;
            u_y += directionVector_[f].second * f_i;
          }
          u_.first = u_x / density_[row][col];
          u_.second = u_y / density_[row][col];
        }

        // Set every f_i to f*
        for (int f = 0; f < 9; f++) {
          d_u = directionVector_[f].first * u_.first +
                directionVector_[f].second * u_.second;

          // d_u = (directionvec bzw c) * u
          //  f = w * p * (1+ 3 d_u + 9/2 d_u **2 -3/2 u**2)
          f_i = grid_[f][row][col];
          grid_[f][row][col] =
              f_i - omega * (f_i - weights_[f] * density_[row][col] *
                                       (1 + 3 * (d_u) + 4.5 * (d_u * d_u) -
                                        1.5 * (u_.first * u_.first +
                                               u_.second * u_.second)));
        }
      }
    }
  }

  void print_grid(size_t function) {
    for (int row = 0; row < rows_; row++) {
      for (int col = 0; col < cols_; col++) {
        std::cout << std::setw(7) << std::fixed << std::setprecision(2)
                  << grid_[function][row][col];
      }
      std::cout << '\n';
    }
    std::cout << "_______________________________________\n";
  }

  void print_density() {
    datatype density;
    for (int row = 0; row < rows_; row++) {

      for (int col = 0; col < cols_; col++) {
        density = 0;
        for (int i = 0; i < 9; i++) {
          density += grid_[i][row][col];
        }

        std::cout << std::setw(7) << std::fixed << std::setprecision(2)
                  << density;
      }
      std::cout << '\n';
    }
    std::cout << "_______________________________________\n";
  }

  datatype get_density(int row, int col,
                       std::vector<std::vector<std::vector<datatype>>> &grid) {

    datatype density = 0;
    for (int i = 0; i < 9; i++) {
      density += grid[i][row][col];
    }
    return density;
  }

  // Returns true if Position is a Fan
  bool is_fan(int row, int col) {
    for (int i = 0; i < fanPositions_.size(); i++) {
      if (row == fanPositions_[i].first and col == fanPositions_[i].second) {
        return true;
      }
    }
    return false;
  }

  bool is_fan_proportional(int row, int col) {
    int mincols = cols_ * 0.20;
    int maxcols = cols_ * 0.30;
    int correct_row = rows_ * 0.5;

    if (row == correct_row and col > mincols and col < maxcols) {
      return true;
    }
    return false;
  }

  // Returns steps needed to return to initial State. The State is one Point
  // filled with 1.0
  int run_cycle() {

    // setup
    std::vector<datatype> vec1(9, 1);
    set_point(1, 1, vec1);
    int timeatStart = dt;
    int CorrectPoints = 0;

    while (true) {
      step();
      CorrectPoints = 0;
      for (int func = 0; func < 9; func++) {
        if (get_point(1, 1, func) == 1) {
          CorrectPoints++;
        }
      }
      if (CorrectPoints == 9) {
        break;
      }
    }
    std::cout << "Time needed for Grid with dim =" << rows_ << " , " << cols_
              << " is " << dt - timeatStart << ".\n";
    return dt - timeatStart;
  }

  void stream_with_bc() {}

  void collision_T_C02(double omega_T = 1.0, double omega_C = 1.0) {
    std::cout << "Start Function \n";
    // T_density = grid_T_[0];
    double T_density = 0;
    double C_Density = 0;
    double ux = 0.0;
    double uy = 0.0;
    double u_ges = 0;

    for (int row = 0; row < rows_; row++) {
      for (int col = 0; col < cols_; col++) {

        T_density = 0;
        C_Density = 0;
        ux = 0;
        uy = 0;

        for (int i = 0; i < 9; i++) {
          T_density += grid_T_[i][row][col];
          C_Density += grid_C02_[i][row][col];
          ux += directionVector_[i].first * grid_[i][row][col];
          uy += directionVector_[i].second * grid_[i][row][col];
        }
        u_ges = get_density(row, col, grid_);
        ux /= u_ges;
        uy /= u_ges;
        for (int i = 0; i < 9; i++) {
          double T_eq = weights_[i] * T_density *
                        (1.0 + 3.0 * directionVector_[i].first * ux +
                         directionVector_[i].second * uy);

          double C_eq = weights_[i] * C_Density *
                        (1.0 + 3.0 * directionVector_[i].first * ux +
                         directionVector_[i].second * uy);

          double T_i = grid_T_[i][row][col];
          double C_i = grid_C02_[i][row][col];

          grid_T_[i][row][col] = grid_T_[i][row][col] - omega_T * (T_i - T_eq);
          grid_C02_[i][row][col] = C_i - omega_C * (C_i - C_eq);
          // std::cout<< "Row/Col" << row << "/" << col << " : " << T_i<< "\n";
        }
      }
    }
  }

  void collision_C02(double omega_T = 1.0) {}
};

class HeatMap {
public:
  HeatMap(unsigned width, unsigned height)
      : m_width(width), m_height(height), pixels_u(width * height * 4),
        pixels_T(width * height * 4), pixels_C(width * height * 4) {
    m_image.create(width, height, sf::Color::Black);
    m_texture.create(width, height);
    m_sprite.setTexture(m_texture);
  }

  void update(SimGrid &grid) {
    return;
    for (int row = 0; row < grid.rows_; row++) {
      for (int col = 0; col < grid.cols_; col++) {
      }
    }
  }

  void setPosition(float x, float y) { m_sprite.setPosition(x, y); }

  void setScale(float sx, float sy) { m_sprite.setScale(sx, sy); }

  void draw(sf::RenderWindow &window) { window.draw(m_sprite); }

private:
  unsigned m_width;
  unsigned m_height;

  std::vector<sf::Uint8> pixels_u;
  std::vector<sf::Uint8> pixels_T;
  std::vector<sf::Uint8> pixels_C;

  sf::Image m_image;
  sf::Texture m_texture;
  sf::Sprite m_sprite;
};