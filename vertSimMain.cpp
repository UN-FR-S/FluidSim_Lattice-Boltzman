#include "ArrayLattice.cpp"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <stdlib.h>

int main() {
  datatype e = 0.1;
  constexpr int N = 400;
  constexpr int ROWS = N;
  constexpr int COLS = N;

  double Pr = 0.71;
  double Ra = 1e7;
   
  double L = 4; // m
  double dT = 20; // K
  double v_lB = 0.04;
  double v_phys = 1.5e-5;
  double Ma = 0.05;
  double us = 0.577350269;  

  double alpha_lB  = v_lB/ Pr;

  double Omega = 1.0 / (v_lB* 3.0 + 0.5); 
  double Omega_T = 1.0 / (alpha_lB * 3.0 + 0.5);

  double beta_g = Ra * v_lB * alpha_lB / (1 * N * N *N );

  double dt = Ma * L / ( N * std::sqrt(Ra * v_phys* v_phys / Pr));

  std::cout<<"Omega_U und OmegaT sind " << Omega << " / " << Omega_T << "  ## \n";
  std::cout<<"Beta-g ist " << beta_g << " ## \n";

  auto full_time = std::chrono::high_resolution_clock::now();
  datatype Omega_C = 1.0;
  datatype Fan_Speed = 0.0005;
  datatype alpha =  4.470750e-02;
  const bool Fan_Toggle = false; 

  SimGrid simGrid = SimGrid(N, N);
  simGrid.set_all_to_standard();
  simGrid.set_to_gradient(simGrid.grid_T_);
  simGrid.set_half_to_O1(simGrid.grid_C02_);
  simGrid.set_bc_to_0(simGrid.grid_);
  simGrid.set_bc_to_0(simGrid.grid_T_);
  simGrid.set_bc_to_0(simGrid.grid_C02_);

  datatype avgstream = 0; 
  datatype avgcollison = 0;
  datatype avgbc = 0.0;
  datatype avgrender = 0.0;
  
  const int count_runs = 30000;
  std::cout<< "Es werden " << count_runs << " Steps berechnet für 10s. \n";

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
    simGrid.fast_collision(Omega, Omega_T, Omega_C, Fan_Speed, alpha_lB, Fan_Toggle,
                           beta_g, 0.0); 
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
    simGrid.lower_Boundary_T();
    simGrid.upper_Boundary_T();
    // simGrid.left_boundary_cond();
    // simGrid.right_boundary_cond();
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
  std::string command =
    "conda run -n FluidEnv python quiver.py \"" +
  frameDir.string() + "\"";
  std::system(command.c_str());


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
