function allocate() {
    data = new Array(1000).fill(0);
    return data;
}

results = [];
for (var i = 0; i < 125; ++i) {
    var t0 = performance.now();
    allocate();
    var stop = performance.now() - t0;
    results.push(stop);
}

mean = 0;
for (var i = 0; i < 125; ++i) {
    mean += results[i];
}
mean /= 125;

console.log(`Mean time ${mean} ms`);

results.sort();

if (results.length % 2 == 0)
    console.log(`Median time: ${(results[Math.floor((results.length / 2) - 1)] + results[Math.floor(results.length / 2)]) / 2.0} ms`);
else
    console.log(`Median time: ${results[Math.floor(results.length / 2)]} ms`);

temp = 0;
for (var i = 0; i < 125; ++i) {
    temp += (results[i] - mean) * (results[i] - mean);
}
stddev = Math.sqrt(temp / 125);

console.log(`Stddev: ${stddev} ms`);