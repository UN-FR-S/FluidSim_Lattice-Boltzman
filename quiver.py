import sys
import re
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


def create_quiver_plots(folder_path):

    folder = Path(folder_path)

    # Alle Quiver_*.csv Dateien finden
    csv_files = sorted(
        folder.glob("Quiver_*.csv"),
        key=lambda file: int(
            re.search(r"Quiver_(\d+)\.csv", file.name).group(1)
        )
    )

    if not csv_files:
        print("Keine Quiver-CSV-Dateien gefunden.")
        return

    # Ausgabeordner
    output_folder = folder / "quiver_png"
    output_folder.mkdir(exist_ok=True)

    for csv_file in csv_files:

        #print(f"Verarbeite {csv_file.name}")

        data = pd.read_csv(csv_file)

        x = data["x"].to_numpy()
        y = data["y"].to_numpy()
        u = data["u"].to_numpy()
        v = data["v"].to_numpy()

        speed = np.sqrt(u**2 + v**2)

        plt.figure(figsize=(12, 8))

        q = plt.quiver(
            x,
            y,
            u,
            v,
            speed,
            cmap="viridis"
        )

        q.set_clim(0.0, 0.001)
        plt.colorbar(q, label="Geschwindigkeit")

        plt.xlabel("x")
        plt.ylabel("y")
        plt.title(csv_file.stem)

        plt.axis("equal")
        plt.tight_layout()

        output_file = output_folder / f"{csv_file.stem}.png"

        plt.savefig(output_file, dpi=200)
        plt.close()

        #print(f"Gespeichert: {output_file}")


if __name__ == "__main__":

    if len(sys.argv) != 2:
        print("Usage:")
        print("python3 quiver.py <folder>")
        sys.exit(1)

    folder_path = sys.argv[1]
    print("Python Starts \n")

    create_quiver_plots(folder_path)



