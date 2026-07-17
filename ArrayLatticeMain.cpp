#include "ArrayLattice.cpp"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <stdlib.h>

int main() {
  datatype e = 0.1;
  constexpr int N = 1000;
  constexpr int ROWS = N;
  constexpr int COLS = N;
  constexpr int CELL_SIZE = 10;
  SimGrid simGrid = SimGrid(N, N);
  simGrid.set_all_to_standard();
  std::vector<datatype> vec1 = {4.0 / 9.0,       1.0f / 9.0f + e, 1.0 / 9.0,
                                1.0f / 9.0f - e, 1.0 / 9.0,       1.0 / 36.0,
                                1.0 / 36.0,      1.0 / 36.0,      1.0 / 36.0};
  simGrid.set_point(1, 1, vec1);
  datatype avgstream = 0;
  datatype avgcollison = 0;

  const int count_runs = 100;
  

  sf::RenderWindow window(sf::VideoMode(COLS * CELL_SIZE, ROWS * CELL_SIZE),
                          "Grid");

  int t = 0;
  while (window.isOpen() and t < count_runs) {
    // simGrid.print_density();
    auto start_col = std::chrono::high_resolution_clock::now();
    simGrid.collision();
    auto end_col = std::chrono::high_resolution_clock::now();
    std::chrono::duration<datatype, std::micro> duration = end_col - start_col;
    avgcollison += duration.count();
    // std::cout << "DauerCollision: " << duration.count() << " micro s\n";

    auto start_step = std::chrono::high_resolution_clock::now();
    simGrid.step();
    auto end_step = std::chrono::high_resolution_clock::now();
    std::chrono::duration<datatype, std::micro> duration_step =
        end_step - start_step;
    // std::cout << "DauerStream: " << duration_step.count() << " micro s\n";
    avgstream += duration_step.count();

    sf::Event event;

    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear(sf::Color::White);
    sf::Uint8 r;
    sf::Uint8 g;
    sf::Uint8 b;

    for (int row = 0; row < ROWS; row++) {
      for (int col = 0; col < COLS; col++) {
        sf::RectangleShape cell;
        double minValue = 0.90;
        double maxValue = 1.10;

        double density = simGrid.get_density(row, col);
        double normalized = (density - minValue) / (maxValue - minValue);

        cell.setSize(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

        cell.setPosition(col * CELL_SIZE, row * CELL_SIZE);

        if (normalized < 0.5) {
          r = 0;
          g = static_cast<sf::Uint8>(normalized * 2 * 255);
          b = static_cast<sf::Uint8>((1 - normalized * 2) * 255);
        } else {
          r = static_cast<sf::Uint8>((normalized - 0.5) * 2 * 255);
          g = static_cast<sf::Uint8>((1 - (normalized - 0.5) * 2) * 255);
          b = 0;
        }

        cell.setFillColor(sf::Color(r, g, b));

        window.draw(cell);
      }
    }

    window.display();
    sf::sleep(sf::seconds(1));
    t++;
  }
  avgstream = avgstream / count_runs;
  avgcollison = avgcollison / count_runs;

  std::cout << "Durchschnittliche Laufzeit Stream bei N = " << N << " : "
            << avgstream << " micro s \n";
  std::cout << "Durschnittliche Laufzeit Collision bei N = " << N << " : "
            << avgcollison << " micro s \n";
  std::cout << "Durchschnittliche Gesamtlaufzeit: " << avgcollison + avgstream
            << " micro s \n";
};
