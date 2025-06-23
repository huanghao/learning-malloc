import math
import matplotlib.pyplot as plt

def f(x):
    return x**2 - 6*x + 15


def gss(f, a, b, tol=1e-5):
    """
    黄金分割搜索算法，在区间 [a, b] 上寻找单峰函数 f(x) 的最小值。
    tol: 容忍误差，区间长度小于 tol 时停止。
    返回：最小值点 x 及其函数值 f(x)
    """
    gr = (math.sqrt(5) - 1) / 2  # 黄金比例系数
    iter_times, func_calls = 0, 0

    c = b - (b - a) * gr
    d = a + (b - a) * gr
    while abs(b - a) > tol:
        if f(c) < f(d):
            b = d
            d = c
            c = b - (b - a) * gr
        else:
            a = c
            c = d
            d = a + (b - a) * gr
        iter_times += 1
        func_calls += 1

    x_min = (b + a) / 2
    return x_min, f(x_min), iter_times, func_calls


def ss(f, a, b, r, tol=1e-5):
    iter_times, func_calls = 0, 0
    r = max(r, 1-r)

    while abs(b - a) > tol:
        c = b - (b - a) * r
        d = a + (b - a) * r
        if f(c) < f(d):
            b = d
        else:
            a = c
        iter_times += 1
        func_calls += 2

    x_min = (b + a) / 2
    return x_min, f(x_min), iter_times, func_calls


if __name__ == "__main__":
    a, b = 0, 5
    x_min, f_min, iter_times, func_calls = gss(f, a, b)
    print(f"最小值点 x ≈ {x_min}")
    print(f"最小值 f(x) ≈ {f_min}")
    print(f"迭代次数: {iter_times}")
    print(f"函数调用次数: {func_calls}")

    xs, y1, y2, y3 = [], [], [], []
    for r in range(1, 10):
        if r == 5:
            r = 0.618
        else:
            r = r / 10
        x_min, f_min, iter_times, func_calls = ss(f, a, b, r)
        print(f"r = {r}")
        print(f"最小值点 x ≈ {x_min}")
        print(f"最小值 f(x) ≈ {f_min}")
        print(f"迭代次数: {iter_times}")
        print(f"函数调用次数: {func_calls}")
        xs.append(r)
        y1.append(iter_times)
        y2.append(func_calls)
        y3.append(iter_times * func_calls)

    plt.plot(xs, y1, label="iter")
    plt.plot(xs, y2, label="calls")
    # plt.plot(xs, y3, label="efficiency")
    plt.legend()
    plt.show()
