import csv
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

path = sys.argv[1] if len(sys.argv) > 1 else "results.csv"
with open(path, encoding="utf-8") as f:
    rows = list(csv.DictReader(f))
if not rows:
    sys.exit("В " + path + " нет данных")

sizes = [int(r["N"]) for r in rows]


def column(name):
    return [float(r[name]) for r in rows]


fig, axes = plt.subplots(1, 2, figsize=(12, 5))
charts = [
    (axes[0], "add", "Вставка одного элемента", "tab:blue"),
    (axes[1], "remove", "Удаление одного элемента", "tab:red"),
]
for ax, key, title, color in charts:
    median = column(key + "_median_ns")
    avg = column(key + "_avg_ns")
    low = column(key + "_min_ns")
    ax.plot(sizes, median, "o-", color=color, linewidth=2, label="медиана")
    ax.plot(sizes, avg, "^:", color=color, alpha=0.7, label="среднее")
    ax.plot(sizes, low, "s--", color=color, alpha=0.4, label="минимум")
    for x, y in zip(sizes, median):
        ax.annotate(f"{y:.0f}", (x, y), textcoords="offset points", xytext=(0, 8), ha="center")
    ax.set_xscale("log")
    ax.set_ylim(bottom=0)
    ax.set_title(title + " в AVL-дерево из N элементов")
    ax.set_xlabel("N, элементов в дереве (логарифмическая шкала)")
    ax.set_ylabel("время одной операции, нс")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend()

fig.tight_layout()
fig.savefig("benchmark.png", dpi=150)
print("График сохранён в benchmark.png")
