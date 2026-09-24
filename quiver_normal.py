
import sys
import re
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


def create_quiver_plots(folder_path):

    folder = Path(folder_path)

    csv_files = sorted(
        folder.glob("Quiver_*.csv"),
        key=lambda file: int(
            re.search(r"Quiver_(\d+)\.csv", file.name).group(1)
        )
    )

    if not csv_files:
        print("Keine Quiver-CSV-Dateien gefunden.")
        return

    output_folder = folder / "quiver_png"
    output_folder.mkdir(exist_ok=True)

    # ---------------------------------------------------------
    # Globales Maximum der Pfeillänge bestimmen
    # ---------------------------------------------------------

    max_speed = 0.0

    for csv_file in csv_files:

        data = pd.read_csv(csv_file)

        u = data["u"].to_numpy()
        v = data["v"].to_numpy()

        # Betrag der Geschwindigkeit
        speed = np.sqrt(u**2 + v**2)

        max_speed = max(
            max_speed,
            np.max(speed)
        )

    print(f"Maximale Geschwindigkeit: {max_speed}")

    # ---------------------------------------------------------
    # Plots erzeugen
    # ---------------------------------------------------------

    for csv_file in csv_files:

        data = pd.read_csv(csv_file)

        x = data["x"].to_numpy()
        y = data["y"].to_numpy()
        u = data["u"].to_numpy()
        v = data["v"].to_numpy()

        # Betrag der Geschwindigkeit für die Farbgebung
        speed = np.sqrt(u**2 + v**2)

        plt.figure(figsize=(12, 8))

        # Farbe wird durch die Pfeillänge bestimmt
        q = plt.quiver(
            x,
            y,
            u/speed,
            v/speed,
            speed,
            cmap="YlOrRd",
            clim=(0, max_speed)
        )

        plt.colorbar(
            q,
            label="Geschwindigkeitsbetrag $|\\vec{u}|$"
        )

        plt.xlabel("x")
        plt.ylabel("y")
        plt.title(csv_file.stem)

        plt.axis("equal")
        plt.gca().invert_yaxis()
        plt.tight_layout()

        output_file = output_folder / f"{csv_file.stem}.png"

        plt.savefig(
            output_file,
            dpi=200
        )

        plt.close()


if __name__ == "__main__":

    if len(sys.argv) != 2:
        print("Usage:")
        print("python3 quiver.py <folder>")
        sys.exit(1)

    folder_path = sys.argv[1]

    print("Python Starts\n")

    create_quiver_plots(folder_path)

