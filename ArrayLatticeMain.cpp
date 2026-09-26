#include "ArrayLattice.cpp"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <stdlib.h>

int main() {
  datatype e = 0.1;
  constexpr int N = 100;
  constexpr int ROWS = N; 
  constexpr int COLS = N;

  double Pr = 0.71;
  double Sc = 0.95;
  
   
  double L = 0.36; // m
  double dT = 5;
  double u_phys = 0.06; // K 
  double u_LB = 0.1;
  
  double v_phys = 1.5e-5;

  double Ma = 0.05;
  double us = 0.577350269;  

  double Re = u_phys * L / v_phys;

  double v_lB = u_LB * N / Re;

  

  double alpha_lB  = v_lB/ Pr;
  double D_C = v_lB / Sc;

  double Omega = 1.0 / (v_lB* 3.0 + 0.5); 
  double Omega_T = 1.0 / (alpha_lB * 3.0 + 0.5);
  double Omega_C = 1.0 /(D_C * 3.0 + 0.5);

  double beta_g = Re * v_lB * alpha_lB / (1 * N * N *N ) *0.0 ;

  double dx = L / N;
  double dt = u_LB*dx  / u_phys;


  std::cout<<"v_LB = " << v_lB << " \n";
  std::cout << "Re = " << Re << "\n";
  std::cout<<"Omega_U, OmegaT und OmegaC sind " << Omega << " / " << Omega_T << " / " << Omega_C << "  ## \n";
  std::cout<<"Beta-g ist " << beta_g << " ## \n";

  auto full_time = std::chrono::high_resolution_clock::now();

  datatype Fan_Speed = u_LB;
  datatype alpha =  4.470750e-02;
  const bool Fan_Toggle = true;  

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
  
  const int count_runs = static_cast<int>(std::round(60.0 / dt));
  std::cout<< "Es werden " << count_runs << " Steps berechnet für "<< static_cast<int>(std::round(double(count_runs) * dt)) <<"s. \n";

  int t = 0;

  // Add the Dimension and Amount of Runs for Reference
  std::string current_date = getCurrentDate() + "_normal_" + std::to_string(N) + "N" +
                             "_" + std::to_string(count_runs) + "R";

  fs::path frameDir = fs::path("frames") / current_date;
  fs::create_directories(frameDir);

  fs::path csvFile = frameDir / "results.csv";
  std::ofstream file(csvFile);



  file << "L, " << L << "\n";
  file << "N, " << N << "\n"; 
  file << "dt, " << dt << "\n";
  file << "dT, " << dT << "\n"; 
  file << "Re, " << Re << "\n";
  file << "Pr, " << Pr << "\n";
  file << "Ma, " << Ma << "\n";
  file << "beta_g, " << beta_g << "\n";
  file << "Omega," << Omega << "\n";
  file << "Omega_T" << Omega_T << "\n";
  file << "Omega_C02" << Omega_C << "\n";
  file << "FanSpeed" << Fan_Speed << "\n"; 
  file << "Alpha" << alpha << "\n";
  file << "\n"; 
  file << "Step,Density,Temperature,Co2\n";
 
  while (t < (count_runs)) {
    

    if (t % 500 == 0) {
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
    //simGrid.lower_Boundary_T();
    //simGrid.upper_Boundary_T();
    simGrid.left_boundary_cond();
    simGrid.right_boundary_cond();
    auto end_bc = std::chrono::high_resolution_clock::now();
    std::chrono::duration<datatype, std::micro> duration_bc = end_bc - start_bc;
    avgbc += duration_bc.count();

    t++;
  } 
  std::string command =
    "conda run -n FluidEnv python quiver_normal.py \"" +
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
