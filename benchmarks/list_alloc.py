import timeit


def list_alloc():
    data = []
    for _ in range(1000):
        data.append(None)


if __name__ == '__main__':
    results = [i / 5 * 1000 for i in timeit.repeat(lambda: list_alloc(), repeat=25, number=5)]
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