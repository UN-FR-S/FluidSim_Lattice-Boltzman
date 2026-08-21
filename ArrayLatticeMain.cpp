#include "ArrayLattice.cpp"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <stdlib.h>

int main() {

  datatype e = 0.1;
  constexpr int N = 1000;
  constexpr int ROWS = N;
  constexpr int COLS = N;

  auto full_time = std::chrono::high_resolution_clock::now();
  datatype Omega = 1.5;
  datatype Omega_T = 1.0;
  datatype Omega_C = 1.0;
  datatype Fan_Speed = 0.1;
  datatype alpha = 0.00001;
  SimGrid simGrid = SimGrid(N, N);
  simGrid.set_all_to_standard();
  simGrid.set_half_to_O1(simGrid.grid_T_);
  simGrid.set_bc_to_0(simGrid.grid_);
  simGrid.set_bc_to_0(simGrid.grid_T_);
  simGrid.set_bc_to_0(simGrid.grid_C02_);
  std::vector<datatype> vec1 = {4.0 / 9.0,   1.0f / 9.0f, 1.0 / 9.0,
                                1.0f / 9.0f, 1.0 / 9.0,   1.0 / 36.0,
                                1.0 / 36.0,  1.0 / 36.0,  1.0 / 36.0};
  simGrid.set_point(1, 1, vec1);
  datatype avgstream = 0;
  datatype avgcollison = 0;
  datatype avgbc = 0.0;
  datatype avgrender = 0.0;

  const int count_runs = 40000;

  int t = 0;

  // Add the Dimension and Amount of Runs for Reference
  std::string current_date = getCurrentDate() + "_" + std::to_string(N) + "N" +
                             "_" + std::to_string(count_runs) + "R";

  fs::path frameDir = fs::path("frames") / current_date;
  fs::create_directories(frameDir);

  fs::path csvFile = frameDir / "results.csv";
  std::ofstream file(csvFile);

  file << "N," << N << "\n";
  file << "GridSize," << N << "x" << N << "\n";
  file << "Omega," << Omega << "\n";
  file << "Omega_T" << Omega_T << "\n";
  file << "Omega_C02" << Omega_C << "\n";
  file << "FanSpeed" << Fan_Speed << "\n";
  file << "Alpha" << alpha << "\n";
  file << "\n";
  file << "Step,Density,Temperature,Co2\n";

  while (t < (count_runs)) {
    auto start_col = std::chrono::high_resolution_clock::now();
    simGrid.fast_collision(Omega, Omega_T, Omega_C, Fan_Speed, alpha);
    auto end_col = std::chrono::high_resolution_clock::now();
    std::chrono::duration<datatype, std::micro> duration = end_col - start_col;
    avgcollison += duration.count();

    auto start_step = std::chrono::high_resolution_clock::now();
    simGrid.step();
    auto end_step = std::chrono::high_resolution_clock::now();
    std::chrono::duration<datatype, std::micro> duration_step =
        end_step - start_step;
    avgstream += duration_step.count();

    auto start_bc = std::chrono::high_resolution_clock::now();
    simGrid.simpleBounceBack_bc(simGrid.grid_);
    simGrid.simpleBounceBack_bc(simGrid.grid_T_);
    simGrid.simpleBounceBack_bc(simGrid.grid_C02_);
    simGrid.left_boundary_cond();
    simGrid.right_boundary_cond();
    auto end_bc = std::chrono::high_resolution_clock::now();
    std::chrono::duration<datatype, std::micro> duration_bc = end_bc - start_bc;
    avgbc += duration_bc.count();

    if (t % 250 == 0) {
      auto start_render = std::chrono::high_resolution_clock::now();
      save_png(simGrid, t, frameDir);

      file << t << "," << simGrid.getAverage(simGrid.grid_) << ","
           << simGrid.getAverage(simGrid.grid_T_) << ","
           << simGrid.getAverage(simGrid.grid_C02_) << "\n";

      auto end_render = std::chrono::high_resolution_clock::now();

      std::chrono::duration<datatype, std::micro> render_d =
          end_render - start_render;
      avgrender += render_d.count();
    }

    t++;
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  avgstream = avgstream / count_runs;
  avgcollison = avgcollison / count_runs;
  avgbc = avgbc / count_runs;
  avgrender = avgrender / count_runs;

  std::cout << "Durchschnittliche Laufzeit Stream bei N = " << N << " : "
            << avgstream << " micro s \n";
  std::cout << "Durschnittliche Laufzeit Collision bei N = " << N << " : "
            << avgcollison << " micro s \n";

  std::cout << "Durschnittliche Laufzeit Boundary bei N = " << N << " : "
            << avgbc << " micro s \n";
  std::cout << "Durschnittliche Laufzeit Grafische Darstellung bei N = " << N
            << " : " << avgrender << " micro s \n";
  std::cout << "Durchschnittliche Gesamtlaufzeit: "
            << avgcollison + avgstream + avgbc + avgrender << " micro s \n";

  std::chrono::duration<datatype> duration = end_time - full_time;

  std::cout << "Gesamtzeit: " << duration.count() << " s. \n";
  std::cout << "Anzahl Cycles: " << t << ".\n";
  std::cout << "Zeit pro Cycle: " << duration.count() / t << " s.\n";

  file.close();
};
