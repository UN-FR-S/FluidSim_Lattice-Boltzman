#include "ArrayLattice.cpp"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <stdlib.h>

int main() {
  datatype e = 0.1;
  constexpr int N = 100;
  constexpr int ROWS = N;
  constexpr int COLS = N;
  datatype Omega = 0.8;
  datatype Fan_Speed = 1;
  SimGrid simGrid = SimGrid(N, N);
  simGrid.set_all_to_standard();
  std::vector<datatype> vec1 = {4.0 / 9.0,   1.0f / 9.0f, 1.0 / 9.0,
                                1.0f / 9.0f, 1.0 / 9.0,   1.0 / 36.0,
                                1.0 / 36.0,  1.0 / 36.0,  1.0 / 36.0};
  simGrid.set_point(1, 1, vec1);
  datatype avgstream = 0;
  datatype avgcollison = 0;

  const int count_runs = 100;

  constexpr int WINDOW_WIDTH = 1920;
  constexpr int WINDOW_HEIGHT = 1080;

  sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
                          "Heatmap");

  std::vector<sf::Uint8> pixels(ROWS * COLS * 4);
  sf::Texture texture;
  texture.create(COLS, ROWS);

  sf::Sprite sprite(texture);

  sprite.setScale(static_cast<float>(WINDOW_WIDTH) / COLS,
                  static_cast<float>(WINDOW_HEIGHT) / ROWS);
  int t = 0;
  while (window.isOpen() and t < count_runs) {
    // simGrid.print_density();
    auto start_col = std::chrono::high_resolution_clock::now();
    simGrid.collision(Omega, Fan_Speed);
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

        double minValue = 0.90;
        double maxValue = 1.10;

        double density = simGrid.get_density(row, col);
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
        int index = 4 * (row * COLS + col);
        pixels[index + 0] = r;
        pixels[index + 1] = g;
        pixels[index + 2] = b;
        pixels[index + 3] = 255;
      }
    }
    texture.update(pixels.data());

    window.clear();

    window.draw(sprite);
    window.display();
    double sleep_time = 0.25 - 1e-6 * duration_step.count();
    if (sleep_time > 0) {
      sf::sleep(sf::seconds(sleep_time));
    }

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
