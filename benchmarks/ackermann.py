import timeit


def ack(m, n):
    if m > 0:
        if n == 0:
            return ack(m - 1, 1)
        else:
            return ack(m - 1, ack(m, n - 1))
    else:
        return n + 1


if __name__ == '__main__':
    results = [i / 5 * 1000 for i in timeit.repeat(lambda: ack(3, 6), repeat=25, number=5)]
    for r in results:
        print(f"Run time: {r}ms")
    
    mean = sum(results) / len(results)
    print(f"Mean time: {mean}ms")

    results = sorted(results)
    median = 0
    if len(results) % 2 == 0:
        median = results[int(len(results) / 2) - 1] + results[int(len(results) / 2)]
        median /= 2
    else:
        median = results[int(len(results) / 2)]
    print(f"Median time: {median}ms")

    stddev = (sum((a - mean) ** 2 for a in results) / 125) ** 0.5
    print(f"Stddev: {stddev}ms")