import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import argparse

from parse_bench import parse_benchmark, parse_bench_name

# ==========================================
# Global settings
# ==========================================
plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['font.sans-serif'] = ['Arial', 'Helvetica', 'sans-serif']
plt.rcParams['axes.unicode_minus'] = False
plt.rcParams['savefig.dpi'] = 300

# Load real data from files
parser = argparse.ArgumentParser()
parser.add_argument('on', type=str, help='bench(sym=on)')
parser.add_argument('off', type=str, help='bench(sym=off)')
args = parser.parse_args()

def get_on_off_param(path1: str, path2: str) -> tuple[list[int],list[int],dict]:
    d1 = parse_bench_name(path1)
    d2 = parse_bench_name(path2)
    sym1, sym2 = d1.get('sym'), d2.get('sym')
    if sym1 == sym2: raise ValueError(f"Both have the same symmetry (sym={sym1})")
    d1_cmp = { k:v for k,v in d1.items() if k != 'sym' }
    d2_cmp = { k:v for k,v in d2.items() if k != 'sym' }
    if d1_cmp != d2_cmp: 
        diffs = {k:(d1_cmp.get(k), d2_cmp.get(k)) for k in d1_cmp.keys() if d1_cmp.get(k)!= d2_cmp.get(k)}
        raise ValueError(f"Both have different parameters:\n{diffs}")
    
    if not sym1: path1, path2 = path2, path1
    _lim1, on = parse_benchmark(path1)
    _lim2, off = parse_benchmark(path2)
    if _lim1 != _lim2 or _lim1 != d1_cmp.get('T'): 
        raise ValueError(f"???corrupted files:\n- {path1}\n- {path2}")

    return on,off,d1_cmp

on_list, off_list, params = get_on_off_param(args.on, args.off)
on = np.asarray(on_list)
off = np.asarray(off_list)
limit = params.get('T')

# Compute ECDF
def calc_ecdf(data):
    x = np.sort(data)
    y = np.arange(1, len(x) + 1) / len(x)
    return x, y

x_on, y_on = calc_ecdf(on)
x_off, y_off = calc_ecdf(off)

# Create canvas & Plot step chart
fig, ax = plt.subplots(figsize=(8.5, 5))
color_on = '#0072BD'
color_off = '#D95319'

ax.step(x_on, y_on, where='post', color=color_on, linewidth=1.0, label='sym=on')
ax.step(x_off, y_off, where='post', color=color_off, linewidth=1.0, label='sym=off')

# Axes and limits
ax.set_xscale('log')
ax.set_xlim(1, limit)
ax.set_ylim(0, 1.0)
ax.yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1.0, decimals=0, symbol=''))

# Grid configuration: Detailed Minor Grid for X
ax.grid(True, which='major', axis='both', linestyle='-', color='#cccccc', linewidth=0.6)
ax.grid(True, which='minor', axis='x', linestyle=':', color='#cccccc', linewidth=0.6)
ax.set_axisbelow(True)

# Labels & Title
ax.set_xlabel('Solve time (ms, log scale)', fontsize=8, labelpad=8)
ax.set_ylabel('Fraction solved within time (%)', fontsize=8, labelpad=8)
ax.set_title("Empirical CDF: Symmetry On vs Off", fontsize=10, weight="heavy", pad=12)

# Ticks configuration
ax.set_yticks(np.arange(0, 1.1, 0.1))
ax.tick_params(axis='both', which='major', labelsize=8, direction='in', 
               length=6, width=1, top=True, right=True)
ax.tick_params(axis='both', which='minor', direction='in', 
               length=3, width=0.8, top=True, right=True)

ax.xaxis.set_major_formatter(ticker.LogFormatterMathtext())

# Spines
for spine in ax.spines.values():
    spine.set_visible(True)
    spine.set_color('black')
    spine.set_linewidth(1.0)

# Legend
legend = ax.legend(loc='upper left', fontsize=8, frameon=True, 
                   facecolor='white', edgecolor='black', framealpha=1.0,
                   borderpad=0.6, labelspacing=0.4, fancybox=False)
legend.get_frame().set_linewidth(1.0)

# Version & params
param_str = f"v{params['ver']}_N{params['N']}_L{params['L']}_G{params['G']}_S{params['S']}_T{params['T']}"
ax.text(0.98, 0.03, param_str, 
        transform=ax.transAxes,
        fontsize=8, 
        color='#555555',            
        ha='right', va='bottom', 
        fontfamily='monospace',  
        bbox=dict(facecolor='white', alpha=0.8, edgecolor='none', pad=2))

# Save and show
plt.tight_layout()
plt.show()