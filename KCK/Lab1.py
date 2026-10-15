import matplotlib.pyplot as plt
import pandas as pd


def plot_50():
    files = ["1c", "1crs", "1ers", "2c", "2crs"]
    names = ["1-Coev", "1-Coeav-RS", "1-Evol-RS", "2-Coev", "2-Coev-RS"]
    plot_data = {
        ("1c", "1-Coev"),
        ("1crs", "1-Coeav-RS"),
        ("1ers", "1-Evol-RS"),
        ("2c", "2-Coev"),
        ("2crs", "2-Coev-RS"),
    }

    data_frames = {}
    fig, ax = plt.subplots(figsize=(9, 5))

    for file, name in enumerate(files):
        data = pd.read_csv(f"Lab1/{file}.csv", sep=",")
        run_col = data.iloc[:, 2:]
        data["mean_run"] = run_col.mean(axis=1)
        data_frames[file] = data
        ax.plot(data["effort"], data["mean_run"], label=f"{name}")

    ax.set_xlim(0, 500000)
    ax.set_ylim(0.60, 1.00)
    ax.set_xlabel("Rozegranych gier")
    ax.set_ylabel("Odsetek wygranych gier")
    ax.legend()
    plt.show()


def plot_100():
    plot_data = {
        "1ers": {"name": "1-Evol-RS", "color": "blue", "marker": "o"},
        "1crs": {"name": "1-Coev-RS", "color": "forestgreen", "marker": "v"},
        "2crs": {"name": "2-Coev-RS", "color": "red", "marker": "D"},
        "1c": {"name": "1-Coev", "color": "black", "marker": "s"},
        "2c": {"name": "2-Coev", "color": "magenta", "marker": "d"},
    }

    fig, (ax1, ax2) = plt.subplots(
        1, 2, figsize=(10, 6), gridspec_kw={"width_ratios": [2, 1.2]}
    )

    box_data = []
    box_labels = []

    for file, atr in plot_data.items():
        data = pd.read_csv(f"Lab1/{file}.csv", sep=",")

        run_col = data.iloc[:, 2:]
        mean_run = run_col.mean(axis=1) * 100
        effort = data["effort"] / 1000

        ax1.plot(
            effort,
            mean_run,
            label=atr["name"],
            color=atr["color"],
            marker=atr["marker"],
            markevery=0.16,
            markersize=6,
            markeredgecolor="black",
            markeredgewidth=0.5,
            linewidth=1.2,
        )

        final_runs = run_col.iloc[-1, :] * 100
        box_data.append(final_runs)
        box_labels.append(atr["name"])

    ax1.set_xlim(0, 500)
    ax1.set_ylim(60, 100)
    ax1.set_xlabel("Rozegranych gier ($\\times 1000$)")
    ax1.set_ylabel("Odsetek wygranych gier [%]")
    ax1.legend(loc="lower right")
    ax1.tick_params(axis="both", direction="in", top=False, right=True)

    ax3 = ax1.twiny()
    ax3.set_xlim(0, 200)
    ax3.set_xticks([0, 40, 80, 120, 160, 200])
    ax3.set_xlabel("Pokolenie")
    ax3.tick_params(direction="in")

    boxprops = dict(color="blue", linewidth=1.5)
    whiskerprops = dict(color="blue", linestyle="--", linewidth=1.5)
    medianprops = dict(color="red", linewidth=1.5)
    flierprops = dict(marker="+", color="blue", markeredgecolor="blue", markersize=8)
    capprops = dict(color="black", linewidth=1.5)
    meanprops = dict(
        marker="o", markerfacecolor="black", markeredgecolor="black", markersize=5
    )

    ax2.boxplot(
        box_data,
        notch=True,
        label=box_labels,
        boxprops=boxprops,
        whiskerprops=whiskerprops,
        medianprops=medianprops,
        flierprops=flierprops,
        capprops=capprops,
        showmeans=True,
        meanprops=meanprops,
    )

    ax2.set_ylim(60, 100)
    ax2.yaxis.tick_right()
    ax2.tick_params(axis="y", direction="in", left=False, right=True)
    ax2.tick_params(axis="x", direction="in")
    ax2.grid(True, linestyle=":", color="gray", alpha=0.7)

    plt.show()


def main():
    plot_100()


if __name__ == "__main__":
    main()
