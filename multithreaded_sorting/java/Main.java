/*
 * Bryce Souers
 * Main.java - Multithreaded sorting program
 */

import java.util.Random;

public class Main extends Thread {
	public static void main(String[] args) {
		if(args.length < 1) {
			System.err.println("[ERROR] Missing argument number.");
			System.exit(1);
		}
		int n = Integer.parseInt(args[0]);
		long ts_begin, ts_end;
		double elapsed;

		Random r = new Random();

		double[] a = new double[n];
		for(int i = 0; i < n; i++) a[i] = 1.0f + (1000.0f - 1.0f) * r.nextDouble();

		//-------------------- ONE THREAD CASE --------------------
		double[] b = new double[n];
		for(int i = 0; i < n; i++) b[i] = a[i];
		ts_begin = System.nanoTime();
		try {
			Sort sort = new Sort(b, n);
			sort.start();
			sort.join();
			b = sort.getData();
		} catch(Exception e) {
			e.printStackTrace();
			System.exit(1);
		}
		ts_end = System.nanoTime();
		elapsed = (ts_end - ts_begin) / 1000000.0f;
		System.out.println("Sorting by ONE thread is done in " + String.format("%.1f", elapsed) + "ms.");

		//-------------------- TWO THREAD CASE --------------------
		double[] a_1 = new double[n / 2];
		double[] a_2 = new double[n / 2];
		for(int i = 0; i < (n / 2); i++) {
			a_1[i] = a[i];
			a_2[i] = a[i + (n / 2)];
		}
		ts_begin = System.nanoTime();
		
		try {
			Sort sort_1 = new Sort(a_1, n / 2);
			sort_1.start();
			sort_1.join();
			a_1 = sort_1.getData();

			Sort sort_2 = new Sort(a_2, n / 2);
			sort_2.start();
			sort_2.join();
			a_2 = sort_2.getData();

			Merge merge = new Merge(a_1, a_2, n);
			merge.start();
			merge.join();
			a = merge.getData();
		} catch(Exception e) {
			e.printStackTrace();
			System.exit(1);
		}

		ts_end = System.nanoTime();
		elapsed = (ts_end - ts_begin) / 1000000.0f;

		System.out.println("Sorting by TWO threads is done in " + String.format("%.1f", elapsed) + "ms.");
	}
}

class Merge extends Thread {
	double a[], a_1[], a_2[];
	int n;
	public Merge(double in_a1[], double in_a2[], int in_n) {
		n = in_n;
		a = new double[n];
		a_1 = new double[n / 2];
		for(int i = 0; i < n / 2; i++) a_1[i] = in_a1[i];
		a_2 = new double[n / 2];
		for(int i = 0; i < n / 2; i++) a_2[i] = in_a2[i];
	}
	public void run() {
		int i = 0, j = 0, k = 0;
		while(i < (n / 2) && j < (n / 2)) {
			if(a_1[i] < a_2[j]) a[k++] = a_1[i++];
			else a[k++] = a_2[j++];
		}
		while(i < (n / 2)) a[k++] = a_1[i++];
		while(j < (n / 2)) a[k++] = a_2[j++];
	}
	public double[] getData() {
		return a;
	}
}

class Sort extends Thread {
	double a[];
	int n;
	public Sort(double in_a[], int in_n) {
		n = in_n;
		a = new double[n];
		for(int i = 0; i < n; i++) a[i] = in_a[i];
	}
	public void run() {
		int i = 0, j = 0, min = 0;
		for(i = 0; i < (n - 1); i++) {
			min = i;
			for(j = (i + 1); j < n; j++) if(a[j] < a[min]) min = j;
			double t = a[min];
			a[min] = a[i];
			a[i] = t;
		}
	}
	public double[] getData() {
		return a;
	}
}
