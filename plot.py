"""График замеров AVL-дерева: время вставки и удаления ОДНОГО элемента в зависимости от N.

Запуск: python3 plot.py [results.csv] [benchmark.png]
Программа Laba1 вызывает его сама после замеров (пункт 5 меню).
"""
import csv
import sys

import matplotlib

matplotlib.use("Agg")  # рисуем сразу в файл, окно не нужно
import matplotlib.pyplot as plt

csv_path = sys.argv[1] if len(sys.argv) > 1 else "results.csv"
png_path = sys.argv[2] if len(sys.argv) > 2 else "benchmark.png"

with open(csv_path, encoding="utf-8") as f:
    rows = list(csv.DictReader(f))
if not rows:
    sys.exit("В файле нет замеров")

n = [int(r["n"]) for r in rows]
insert_ns = [float(r["insert_ns"]) for r in rows]
remove_ns = [float(r["remove_ns"]) for r in rows]

plt.figure(figsize=(9, 5.5))
plt.plot(n, insert_ns, marker="o", linewidth=2, label="вставка одного элемента")
plt.plot(n, remove_ns, marker="s", linewidth=2, label="удаление одного элемента")

for x, y in zip(n, insert_ns):
    plt.annotate(f"{y:.0f}", (x, y), textcoords="offset points", xytext=(0, 8), ha="center", fontsize=8)
for x, y in zip(n, remove_ns):
    plt.annotate(f"{y:.0f}", (x, y), textcoords="offset points", xytext=(0, -14), ha="center", fontsize=8)

plt.xscale("log")
plt.xlabel("Количество элементов в дереве N (логарифмическая шкала)")
plt.ylabel("Среднее время одной операции, нс")
total = rows[0].get("measurements", "?")
plt.title(f"AVL-дерево из N элементов: вставка и удаление одного элемента\n(среднее по {total} замерам на каждое N)")
plt.grid(True, which="both", alpha=0.3)
plt.ylim(bottom=0)
plt.legend()
plt.tight_layout()
plt.savefig(png_path, dpi=130)
print(f"  График сохранён в {png_path}")
