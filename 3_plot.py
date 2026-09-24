
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from matplotlib.colors import LinearSegmentedColormap, Normalize
from matplotlib.cm import ScalarMappable
from pathlib import Path


# ============================================================
# Einstellungen
# ============================================================

folder = Path("frames/2026-09-14_09-31_400N_250000R")

# Hier nur den Zeitpunkt ändern
t = 200000

# Physikalische Ausdehnung des Raumes
Lx = 4.0
Ly = 4.0

# Farbskala aus der C++-Implementierung
min_value = 0.0
max_value = 1.1


# ============================================================
# Dateinamen
# ============================================================

files = [
    folder / f"frame_U_{t}.png",
    folder / f"frame_T_{t}.png",
    folder / f"frame_CO2_{t}.png"
]

titles = [
    r"Fluid pressure",
    r"Temperature",
    r"CO$_2$ concentration"
]


# ============================================================
# Colormap entsprechend der C++-Logik
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
# Abbildung
# ============================================================

fig, axes = plt.subplots(
    1,
    3,
    figsize=(12, 4)
)


# ============================================================
# Heatmaps
# ============================================================

for ax, file, title in zip(axes, files, titles):

    if not file.exists():
        raise FileNotFoundError(
            f"Datei nicht gefunden: {file}"
        )

    img = mpimg.imread(file)

    ax.imshow(
        img,
        extent=[0, Lx, 0, Ly],
        origin="lower",
        aspect="equal"
    )

    ax.set_title(title)

    ax.set_xlabel(r"$x$ [m]")
    ax.set_ylabel(r"$y$ [m]")


# ============================================================
# Gemeinsame Colorbar GANZ RECHTS
# ============================================================

sm = ScalarMappable(
    norm=norm,
    cmap=cmap
)

sm.set_array([])

cbar = fig.colorbar(
    sm,
    ax=axes,
    location="right",
    fraction=0.025,
    pad=0.04
)

cbar.set_label(r"Value")


# ============================================================
# Gesamttitel
# ============================================================

fig.suptitle(
    rf"Simulation at $t = {t}$",
    fontsize=14
)


# ============================================================
# Layout
# ============================================================

#plt.tight_layout()

plt.savefig(
    f"comparison_{t}.png",
    dpi=300,
    bbox_inches="tight"
)

plt.show()

