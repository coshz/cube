import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

limit = 10000

# 1. Load real data from files
on_file  = 'asset/reports/v0051-data_SYMT_N100_L30_G22_S1337_T10000.txt'
off_file = 'asset/reports/v0051-data_SYMF_N100_L30_G22_S1337_T10000.txt'

on  = np.loadtxt(on_file, delimiter=',')
off = np.loadtxt(off_file, delimiter=',')

# 2. Compute ECDF
def calc_ecdf(data):
    x = np.sort(data)
    y = np.arange(1, len(x) + 1) / len(x)
    return x, y

x_on, y_on = calc_ecdf(on)
x_off, y_off = calc_ecdf(off)

# ==========================================
# 3. Global typography settings
# ==========================================
# Force Arial as primary, since it maps bold weights perfectly
plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['font.sans-serif'] = ['Arial', 'Helvetica', 'sans-serif']
plt.rcParams['axes.unicode_minus'] = False

# 4. Create canvas
fig, ax = plt.subplots(figsize=(8.5, 5))

# 5. Plot step chart
color_on = '#0072BD'
color_off = '#D95319'
ax.step(x_on, y_on, where='post', color=color_on, linewidth=1.0, 
        label=f'sym=on ({np.sum(on <= limit)}/{len(on)})')
ax.step(x_off, y_off, where='post', color=color_off, linewidth=1.0, 
        label=f'sym=off ({np.sum(off <= limit)}/{len(off)})')

# 6. Axes and limits
ax.set_xscale('log')
ax.set_xlim(1, limit)
ax.set_ylim(0, 1.0)

# 7. Grid configuration: Detailed Minor Grid for X
ax.grid(True, which='major', axis='both', linestyle='-', color='#cccccc', linewidth=0.6)
ax.grid(True, which='minor', axis='x', linestyle=':', color='#cccccc', linewidth=0.6)
ax.set_axisbelow(True)

# 8. Labels & Title (Applying the explicit bold font property)
ax.set_xlabel('Solve time (ms, log scale)', fontsize=8, labelpad=8)
ax.set_ylabel('Fraction solved within time', fontsize=8, labelpad=8)
# Force the title to use the bold FontProperties object
ax.set_title("Kociemba's Solver ECDF", fontsize=12, weight="heavy", pad=12)

# 9. Ticks configuration
ax.set_yticks(np.arange(0,1.1,0.1))
ax.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.1f'))
ax.tick_params(axis='both', which='major', labelsize=8, direction='in', 
               length=6, width=1, top=True, right=True)
ax.tick_params(axis='both', which='minor', direction='in', 
               length=3, width=0.8, top=True, right=True)

ax.xaxis.set_major_formatter(ticker.LogFormatterMathtext())

# 10. Spines
for spine in ax.spines.values():
    spine.set_visible(True)
    spine.set_color('black')
    spine.set_linewidth(1.0)

# 11. Legend
legend = ax.legend(loc='upper left', fontsize=8, frameon=True, 
                   facecolor='white', edgecolor='black', framealpha=1.0,
                   borderpad=0.6, labelspacing=0.4, fancybox=False)
legend.get_frame().set_linewidth(1.0)

# 12. Save and show
plt.tight_layout()
plt.savefig('out.png', dpi=300, bbox_inches='tight')
plt.show()