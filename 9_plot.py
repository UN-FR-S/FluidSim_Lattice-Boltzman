
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from matplotlib.colors import LinearSegmentedColormap, Normalize
from matplotlib.cm import ScalarMappable
from pathlib import Path


# ============================================================
# Einstellungen
# ============================================================

folder = Path("frames/2026-09-15_14-06_normal_400N_150000R")

# Gewünschte Zeitpunkte / Iterationen
times = [0, 30000, 120000]

# Physikalische Ausdehnung
Lx = 4.0
Ly = 4.0

# Gemeinsame Farbskala
min_value = 0.0
max_value = 1.1


# ============================================================
# Eigene Colormap wie in C++
# ============================================================

cmap = LinearSegmentedColormap.from_list(
    "sfml_colormap",
    [
        (0.0, (0.0, 0.0, 1.0)),   # blau
        (0.5, (0.0, 1.0, 0.0)),   # grün
        (1.0, (1.0, 0.0, 0.0))    # rot
    ]
)

norm = Normalize(
    vmin=min_value,
    vmax=max_value
)


# ============================================================
# 3 × 3 Subplots
# ============================================================

fig, axes = plt.subplots(
    3,
    3,
    figsize=(11, 10)
)


# ============================================================
# Dateien und Darstellung
# ============================================================

prefixes = [
    "frame_U",
    "frame_T",
    "frame_CO2"
]

row_titles = [
    r"$\rho$",
    r"$T$",
    r"$CO_2$"
]


for row in range(3):

    for col in range(3):

        t = times[col]

        file = folder / f"{prefixes[row]}_{t}.png"

        if not file.exists():
            raise FileNotFoundError(
                f"Datei nicht gefunden: {file}"
            )

        img = mpimg.imread(file)

        axes[row, col].imshow(
            img,
            extent=[0, Lx, 0, Ly],
            origin="upper",
            aspect="equal"
        )

        # --------------------------------------------
        # Zeitpunkt nur oben anzeigen
        # --------------------------------------------

        if row == 0:
            axes[row, col].set_title(
                rf"$n = {t}$",
                fontsize=12
            )

        # --------------------------------------------
        # X-Achse nur unten
        # --------------------------------------------

        if row == 2:
            axes[row, col].set_xlabel(r"$x$ [m]")
        else:
            axes[row, col].set_xticklabels([])

        # --------------------------------------------
        # Y-Achse nur links
        # --------------------------------------------

        if col == 0:
            axes[row, col].set_ylabel(r"$y$ [m]")
        else:
            axes[row, col].set_yticklabels([])


# ============================================================
# Bezeichnung der drei Zeilen
# ============================================================

fig.text(
    0.035, 0.77,
    r"$\rho$",
    rotation=90,
    va="center",
    ha="center",
    fontsize=13
)

fig.text(
    0.035, 0.50,
    r"$T$",
    rotation=90,
    va="center",
    ha="center",
    fontsize=13
)

fig.text(
    0.035, 0.23,
    r"$CO_2$",
    rotation=90,
    va="center",
    ha="center",
    fontsize=13
)


# ============================================================
# Eigene Achse für Colorbar
# ============================================================

# [left, bottom, width, height]
cbar_ax = fig.add_axes(
    [0.92, 0.12, 0.02, 0.76]
)


sm = ScalarMappable(
    norm=norm,
    cmap=cmap
)

sm.set_array([])


cbar = fig.colorbar(
    sm,
    cax=cbar_ax
)

'''cbar.set_label(
    "Wert",
    rotation=90
)'''


# ============================================================
# Gesamttitel
# ============================================================

fig.suptitle(
    "Simulation für $N = 400$",
    fontsize=14
)


# ============================================================
# Layout
# ============================================================

plt.subplots_adjust(
    left=0.08,
    right=0.88,
    bottom=0.08,
    top=0.90,
    wspace=0.05,
    hspace=0.08
)


# ============================================================
# Speichern
# ============================================================

plt.savefig(
    "Grafiken/simulation_comparison.png",
    dpi=300,
    bbox_inches="tight"
)

plt.savefig(
    "Grafiken/simulation_comparison.pdf",
    bbox_inches="tight"
)


plt.show()

