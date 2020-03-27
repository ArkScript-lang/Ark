import java.util.Arrays;

public class Ackermann {
    public static int ack(int m, int n) {
        if (m > 0) {
            if (n == 0) {
                return ack(m - 1, 1);
            } else {
                return ack(m - 1, ack(m, n - 1));
            }
        } else {
            return n + 1;
        }
    }

    public static void main(String args[]) {
        long[] results = new long[125];

        for (int i=0; i < 125; ++i) {
            long startTime = System.nanoTime() / 1000000;
            ack(3, 6);
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