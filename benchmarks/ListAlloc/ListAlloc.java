import java.util.Arrays;

public class ListAlloc {
    public static void allocate() {
        int arr[] = new int[1000];
        Arrays.fill(arr, 0);
    }

    public static void main(String args[]) {
        long[] results = new long[125];

        for (int i=0; i < 125; ++i) {
            long startTime = System.nanoTime() / 1000000;
            allocate();
            long stopTime = System.nanoTime() / 1000000;
            results[i] = stopTime - startTime;
        }

        double mean = 0.0;
        for (long a : results)
            mean += a;
        mean /= 125;

        System.out.println("Mean time: " + mean + "ms");

        Arrays.sort(results);
        if (results.length % 2 == 0)
            System.out.println("Median time: " + (results[(results.length / 2) - 1] + results[results.length / 2]) / 2.0 + "ms");
        else
            System.out.println("Median time: " + results[results.length / 2] + "ms");

        double temp = 0.0;
        for (long a : results)
            temp += (a - mean) * (a - mean);
        double stddev = Math.sqrt(temp / 125);

        System.out.println("Stddev: " + stddev + "ms");
    }
}