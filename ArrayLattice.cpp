
#include <SFML/Graphics.hpp>
#include <algorithm> // std::rotate
#include <filesystem>
#include <fstream>
#include <iomanip> // Print_grid uses set_w
#include <iostream>
#include <math.h>
#include <omp.h>
#include <thread>
#include <vector>

using datatype = double;
namespace fs = std::filesystem;

class SimGrid {
public:
  // 9D Vector containing 2D fields. In these the
  std::vector<std::vector<std::vector<datatype>>> grid_;
  std::vector<std::vector<std::vector<datatype>>> grid_T_;
  std::vector<std::vector<std::vector<datatype>>> grid_C02_;
  const std::vector<std::pair<int, int>> directionVector_;
  std::vector<std::pair<int, int>> fanPositions_;
  const std::vector<datatype> weights_;
  std::vector<std::vector<datatype>> density_;
  std::vector<std::vector<datatype>> T_density;
  std::vector<std::vector<datatype>> C02_density;
  std::pair<datatype, datatype> u_;
  const size_t rows_;
  const size_t cols_;
  std::vector<std::vector<std::pair<datatype, datatype>>> grad_T;
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
        grad_T(r, std::vector<std::pair<datatype, datatype>>(c, {0.0, 0.0})),
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

  void set_bc_to_0(std::vector<std::vector<std::vector<datatype>>> &grid) {
    for (int col = 0; col < cols_; col++) {
      for (int i = 0; i < 9; i++) {
        grid[i][0][col] = 0.0;
        grid[i][rows_ - 1][col] = 0.0;
      }
    }

    for (int row = 1; row < rows_ - 1; row++) {
      for (int i = 0; i < 9; i++) {
        grid[i][row][0] = 0.0;
        grid[i][row][cols_ - 1] = 0.0;
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

  void set_to_gradient(std::vector<std::vector<std::vector<datatype>>> &grid){
    datatype val = 4.0 / 9.0;
    for (int func = 0; func < 9; func++) {
      if (func > 0 and func <= 4) {
        val = 1.0 / 9.0;
      } else if (func > 4) {
        val = 1.0 / 36.0;
      }

      for (int row = 0; row < rows_; row++) {
        for (int col = 0; col < cols_; col++) {
          grid[func][row][col] = row/rows_ * 0.9 + 0.1;
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

    for (int row = 0; row < rows_; row++) {
      if (shift < 0) {
        std::rotate(grid[function][row].begin(),
                    grid[function][row].begin() - shift,
                    grid[function][row].end());
      } else {
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
#pragma omp parallel for schedule(static)
    for (int f = 1; f < 9; f++) {
      shift(directionVector_[f].first, directionVector_[f].second, f);
    }
    dt++;
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

  void
  simpleBounceBack_bc(std::vector<std::vector<std::vector<datatype>>> &grid) {
    // Channel 1 and 3

    for (int row = 0; row < rows_; row++) {
      datatype tmp;
      tmp = grid[1][row][1];
      grid[1][row][1] = grid[3][row][0];
      grid[3][row][0] = tmp;

      tmp = grid[1][row][cols_ - 1];
      grid[1][row][cols_ - 1] = grid[3][row][cols_ - 2];
      grid[3][row][cols_ - 2] = tmp;
    }
    // Channel 2 and 4

    for (int col = 0; col < cols_; col++) {
      datatype tmp;
      tmp = grid[2][0][col];
      grid[2][0][col] = grid[4][1][col];
      grid[4][1][col] = tmp;

      tmp = grid[2][rows_ - 2][col];
      grid[2][rows_ - 2][col] = grid[4][rows_ - 1][col];
      grid[4][rows_ - 1][col] = tmp;
    }

    // Channel 5 and 7:

    for (int row = 2; row < rows_; row++) {
      datatype tmp;
      tmp = grid[7][row][0];
      grid[7][row][0] = grid[5][row - 1][1];
      grid[5][row - 1][1] = tmp;
    }

    for (int col = 1; col < cols_ - 2; col++) {
      datatype tmp;
      tmp = grid[7][rows_ - 1][col];
      grid[7][rows_ - 1][col] = grid[5][rows_ - 2][col + 1];
      grid[5][rows_ - 2][col + 1] = tmp;
    }

    for (int col = 2; col < cols_; col++) {
      datatype tmp;
      tmp = grid[5][0][col];
      grid[5][0][col] = grid[7][1][col - 1];
      grid[7][1][col - 1] = tmp;
    }

    for (int row = 1; row < rows_ - 2; row++) {
      datatype tmp;
      tmp = grid[5][row][cols_ - 1];
      grid[5][row][cols_ - 1] = grid[7][row + 1][cols_ - 2];
      grid[7][row + 1][cols_ - 2] = tmp;
    }

    // Channel 6 and 8

    for (int col = 0; col < cols_ - 2; col++) {
      datatype tmp;
      tmp = grid[6][0][col];
      grid[6][0][col] = grid[8][1][col + 1];
      grid[8][1][col + 1] = tmp;
    }

    for (int row = 1; row < rows_ - 2; row++) {
      datatype tmp = grid[6][row][0];
      grid[6][row][0] = grid[8][row + 1][1];
      grid[8][row + 1][1] = tmp;
    }

    for (int col = 2; col < cols_; col++) {
      datatype tmp = grid[8][rows_ - 1][col];
      grid[8][rows_ - 1][col] = grid[6][rows_ - 2][col - 1];
      grid[6][rows_ - 2][col - 1] = tmp;
    }

    for (int row = 2; row < rows_ - 1; row++) {
      datatype tmp = grid[8][row][cols_ - 1];
      grid[8][row][cols_ - 1] = grid[6][row - 1][cols_ - 2];
      grid[6][row - 1][cols_ - 2] = tmp;
    }
  }

  void left_boundary_cond() {
    double T = 0.8;
    double Pressure = 0.9;
    double C02 = 0.1;

    int upper_limit = 0.55 * rows_;
    int lower_limit = 0.80 * rows_;

    for (int row = upper_limit; row < lower_limit; row++) {
      grid_[5][row - 1][1] = Pressure * weights_[5];
      grid_T_[5][row - 1][1] = T * weights_[5];
      grid_C02_[5][row - 1][1] = C02 * weights_[5];

      grid_[1][row][1] = Pressure * weights_[1];
      grid_T_[1][row][1] = T * weights_[1];
      grid_C02_[1][row][1] = C02 * weights_[1];

      grid_[8][row + 1][1] = Pressure * weights_[8];
      grid_T_[8][row + 1][1] = T * weights_[8];
      grid_C02_[8][row + 1][1] = C02 * weights_[8];
    }
  }

  void right_boundary_cond() {
    double T = 1.2;
    double Pressure = 1.1;
    double C02 = 0.8;

    int upper_limit = 0.1 * rows_;
    int lower_limit = 0.2 * rows_;
    for (int row = upper_limit; row < lower_limit; row++) {
      grid_[6][row - 1][cols_ - 2] = Pressure * weights_[6];
      grid_T_[6][row - 1][cols_ - 2] = T * weights_[6];
      grid_C02_[6][row - 1][cols_ - 2] = C02 * weights_[6];

      grid_[2][row][cols_ - 2] = Pressure * weights_[2];
      grid_T_[2][row][cols_ - 2] = T * weights_[2];
      grid_C02_[2][row][cols_ - 2] = C02 * weights_[2];

      grid_[7][row + 1][cols_ - 2] = Pressure * weights_[7];
      grid_T_[7][row + 1][cols_ - 2] = T * weights_[7];
      grid_C02_[7][row + 1][cols_ - 2] = C02 * weights_[7];
    }
  }

  void upper_Boundary_T() {

    double T = 0.1;

    for (int col = 2; col < cols_ - 3; col++) {
      grid_T_[7][1][col - 1] = T * weights_[7];
      grid_T_[4][1][col] = T * weights_[4];
      grid_T_[6][1][col + 1] = T * weights_[6];
    }
  }

  void lower_Boundary_T() {
    double T = 1.0;
    double U = 0.0;

    for (int col = 2; col < cols_ - 3; col++) {
      grid_T_[6][rows_ - 2][col - 1] = T * weights_[6];
      grid_T_[2][rows_ - 2][col] = T * weights_[2];
      grid_T_[5][rows_ - 2][col + 1] = T * weights_[5];
    }
  }

  void fast_collision(double omega = 1.0, double omega_T = 1.0,
                      double omega_C = 1.0, datatype fan_speed = 1.0,
                      datatype alpha = 0.01, bool fan_on = true,
                      double beta = 0.1 ,double g = 2.509804e-05) {

    // Finding Gradient T
#pragma omp parallel for schedule(static)
    for (int row = 1; row < rows_ - 1; row++) {
      double dTdx = 0.0;
      double dTdy = 0.0;
      double T_i;
      double weight = 0;
      std::pair<int, int> dir_vec;
      for (int col = 1; col < cols_ - 1; col++) {
        dTdx = 0.0;
        dTdy = 0.0;
        for (int i = 0; i < 9; i++) {
          dir_vec = directionVector_[i];
          T_i = grid_T_[i][row + dir_vec.second][col + dir_vec.first];
          weight = weights_[i];
          dTdx += weight * dir_vec.first * T_i;
          dTdy += weight * dir_vec.second * T_i;
        }
        grad_T[row][col] = {3.0 * dTdx, 3.0 * dTdy};
      }
    }

#pragma omp parallel for schedule(static)
    for (int row = 0; row < rows_; row++) {
      double T_density = 0;
      double T_eq = 0;

      double C_Density = 0;
      double C_eq = 0;

      double ux = 0.0;
      double uy = 0.0;
      double u_ges = 0;
      double d_u = 0;

      double f_i = 0;
      double T_i = 0;
      double C_i = 0;

      double u_val = 0;
      for (int col = 0; col < cols_; col++) {

        T_density = 0;
        C_Density = 0;
        ux = 0;
        uy = 0;
        u_ges = 0;
        u_val = 0;

        if (is_fan_proportional(row, col) and fan_on) {
          ux = 0;
          uy = fan_speed;
          for (int i = 0; i < 9; i++) {
            T_density += grid_T_[i][row][col];
            C_Density += grid_C02_[i][row][col];
            u_val = grid_[i][row][col];
            u_ges += u_val;
          }
        } else {
          for (int i = 0; i < 9; i++) {
            T_density += grid_T_[i][row][col];
            C_Density += grid_C02_[i][row][col];
            u_val = grid_[i][row][col];
            ux += directionVector_[i].first * u_val;
            uy += directionVector_[i].second * u_val;
            u_ges += u_val;
          }
        }
        if (fabs(u_ges - 0.0) > 0.00001) {
          ux /= u_ges;
          uy /= u_ges;
          double T = get_density(row,col,grid_T_);
          //ux += alpha * grad_T[row][col].first;
          //uy += alpha * grad_T[row][col].second;
          // p0 Referenzdichte Luft bei 20Grad und 1Atm
          // T_ref ist Mitte zwischen 0.1 und 1.0
           double F_y = beta  * ( T - 0.5) / (u_ges * omega);
           uy += F_y;

        }

        for (int i = 0; i < 9; i++) {
          //double F_T = 3.0 * beta * weights_[i] * (get_density(row, col, grid_T_)- 0.5 ) * u_ges * directionVector_[i].second;
          d_u =
              directionVector_[i].first * ux + directionVector_[i].second * uy;

          T_eq = weights_[i] * T_density * (1.0 + 3.0 * d_u);

          C_eq = weights_[i] * C_Density * (1.0 + 3.0 * d_u);

          T_i = grid_T_[i][row][col];
          C_i = grid_C02_[i][row][col];
          f_i = grid_[i][row][col];

          grid_T_[i][row][col] = T_i - omega_T * (T_i - T_eq);
          grid_C02_[i][row][col] = C_i - omega_C * (C_i - C_eq);
          grid_[i][row][col] =
              f_i - omega * (f_i - weights_[i] * u_ges *
                                       (1 + 3 * (d_u) + 4.5 * (d_u * d_u) -
                                        1.5 * (ux * ux + uy * uy)));
        }
      }
    }
  }

  double getAverage(std::vector<std::vector<std::vector<datatype>>> &grid) {
    double average = 0;
    for (int row = 1; row < rows_ - 1; row++) {
      for (int col = 1; col < cols_ - 1; col++) {
        for (int f = 0; f < 9; f++) {
          average += grid[f][row][col];
        }
      }
    }
    average = average / ((rows_ - 1) * (cols_ - 1));
    return average;
  }

  std::pair<double, double> get_u(int row, int col) {
    double u_val = 0.0;
    double u_x = 0.0;
    double u_y = 0.0;
    double u_ges = 0.0;
    for (int i = 0; i < 9; i++) {
      u_val = grid_[i][row][col];
      u_x += directionVector_[i].first * u_val;
      u_y += directionVector_[i].second * u_val;
      u_ges += u_val;
    }
    if (u_ges == 0.0) {
      std::cout << "rho = 0 at "
                  << row << ", " << col << "\n";


    return {0.0, 0.0};
  }
    u_x = u_x / u_ges;
    u_y = u_y / u_ges;
    return {u_x,u_y} ;
  }

};

std::string getCurrentDate() {
  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);

  std::tm tm{};
  localtime_r(&time, &tm); // Linux

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d_%H-%M");

  return oss.str();
}

void save_png(SimGrid &simGrid, int t, fs::path &frameDirectory) {
  sf::Image image;
  image.create(simGrid.cols_, simGrid.rows_);

  std::string quiver_adress = "Quiver_" + std::to_string(t) + ".csv";
  fs::path csvFile = frameDirectory / quiver_adress;

  std::ofstream file(csvFile);
  file << "x" << "," << "y" << "," << "u" << "," << "v\n";
  double minValue = 0.0;
  double maxValue = 1.1;
#pragma omp parallel for schedule(static)
  for (int row = 0; row < simGrid.rows_; row++) {
    sf::Uint8 r;
    sf::Uint8 g;
    sf::Uint8 b;
    for (int col = 0; col < simGrid.cols_; col++) {

      
      double density = simGrid.get_density(row, col, simGrid.grid_C02_);
      double normalized = (density - minValue) / (maxValue - minValue);
      if (normalized < 0.5) {
        r = 0;
        g = static_cast<sf::Uint8>(normalized * 2 * 255);
        b = static_cast<sf::Uint8>((1 - normalized * 2) * 255);
      } else {
        r = static_cast<sf::Uint8>((normalized - 0.5) * 2 * 255);
        g = static_cast<sf::Uint8>((1 - (normalized - 0.5) * 2) * 255);
        b = 0;
      }
      image.setPixel(col, row, sf::Color(r, g, b));
    }
  }

  for (int row = 1; row < simGrid.rows_-1; row  += simGrid.rows_ / 75) {
    for (int col = 1; col < simGrid.cols_-1; col += simGrid.cols_ / 75) {
      std::pair<double, double> u = simGrid.get_u(row, col);
      file << col << "," << row << "," << u.first << "," << u.second << "\n";
    }}

  std::string frame = "frame_" + std::to_string(t) + ".png";
  image.saveToFile(frameDirectory / frame);
  file.close();
}
