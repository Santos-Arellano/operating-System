# scripts/plot_results.py
#/Users/santosa/Documents/GitHub/operating-System/T22-Rendimiento/scripts/plot_results.py
# Lee ./dat/*.dat y produce summary.csv + gráficas (tiempo, speedup, eficiencia).
import os, re, math, csv, glob
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt

DAT_DIR = Path("dat")
OUT_DIR = Path("out")
OUT_DIR.mkdir(exist_ok=True, parents=True)

def parse_dat_file(path: Path):
    meta = {"executable": None, "N": None, "T": None, "mean": None, "stdev": None, "cv": None}
    times = []
    with open(path, "r") as f:
        for line in f:
            if line.startswith("#"):
                if "executable=" in line: meta["executable"] = line.split("=",1)[1].strip()
                if "size=" in line:        meta["N"] = int(line.split("=",1)[1].strip())
                if "threads=" in line:     meta["T"] = int(line.split("=",1)[1].strip())
                if "mean=" in line:        meta["mean"] = float(line.split("=",1)[1].strip())
                if "stdev=" in line:       meta["stdev"] = float(line.split("=",1)[1].strip())
                if "cv=" in line:          meta["cv"] = float(line.split("=",1)[1].strip())
            else:
                s = line.strip()
                if s and s.lower()!="nan":
                    try:
                        times.append(float(s))
                    except:
                        pass
    return meta, times

rows = []
for p in glob.glob(str(DAT_DIR / "*.dat")):
    meta, times = parse_dat_file(Path(p))
    if meta["executable"] and meta["N"] and meta["T"]:
        rows.append({
            "exec": Path(meta["executable"]).name,
            "N": meta["N"],
            "T": meta["T"],
            "mean_us": meta["mean"],
            "stdev_us": meta["stdev"],
            "cv": meta["cv"],
            "reps": len(times)
        })

if not rows:
    print("No hay .dat en ./dat. Corre primero ./lanzador_santos.pl")
    raise SystemExit(0)

df = pd.DataFrame(rows).sort_values(["exec","N","T"])
# Calcular speedup y eficiencia por (exec, N)
df["speedup"] = None
df["efficiency"] = None

for exec_name, g in df.groupby(["exec","N"]):
    t1 = None
    g_sorted = g.sort_values("T")
    for _, r in g_sorted.iterrows():
        if r["T"] == 1:
            t1 = r["mean_us"]
            break
    if t1 is None:
        continue
    idxs = g.index
    for idx in idxs:
        tp = df.loc[idx, "mean_us"]
        p  = df.loc[idx, "T"]
        df.loc[idx, "speedup"] = t1 / tp if tp and tp>0 else None
        df.loc[idx, "efficiency"] = (t1 / tp) / p if tp and tp>0 else None

OUT_DIR.mkdir(exist_ok=True)
df.to_csv(OUT_DIR / "summary.csv", index=False)

# Graficas por ejecutable
for exec_name, g in df.groupby("exec"):
    # Tiempo vs N (una curva por T)
    plt.figure()
    for T, gg in g.groupby("T"):
        gg2 = gg.sort_values("N")
        plt.plot(gg2["N"], gg2["mean_us"], marker="o", label=f"T={T}")
    plt.xlabel("Tamaño N")
    plt.ylabel("Tiempo (us)")
    plt.title(f"Tiempo — {exec_name}")
    plt.legend()
    plt.grid(True, linestyle=":")
    plt.tight_layout()
    plt.savefig(OUT_DIR / f"time_{exec_name}.png", dpi=160)
    plt.close()

    # Speedup vs T (una curva por N)
    plt.figure()
    for N, gg in g.groupby("N"):
        gg2 = gg.sort_values("T")
        plt.plot(gg2["T"], gg2["speedup"], marker="o", label=f"N={N}")
    plt.xlabel("Hilos/Procesos (T)")
    plt.ylabel("Speedup (T1/Tp)")
    plt.title(f"Speedup — {exec_name}")
    plt.legend()
    plt.grid(True, linestyle=":")
    plt.tight_layout()
    plt.savefig(OUT_DIR / f"speedup_{exec_name}.png", dpi=160)
    plt.close()

    # Eficiencia vs T
    plt.figure()
    for N, gg in g.groupby("N"):
        gg2 = gg.sort_values("T")
        plt.plot(gg2["T"], gg2["efficiency"], marker="o", label=f"N={N}")
    plt.xlabel("Hilos/Procesos (T)")
    plt.ylabel("Eficiencia (S/T)")
    plt.title(f"Eficiencia — {exec_name}")
    plt.legend()
    plt.grid(True, linestyle=":")
    plt.tight_layout()
    plt.savefig(OUT_DIR / f"eff_{exec_name}.png", dpi=160)
    plt.close()

print("Listo. Revisa ./out/summary.csv y PNGs en ./out/")
