

public class TestLamda {

	interface MyLamda {
		int add(int a);
		//int add(int a, int b); //there doesn't exceed 2 abstract methods
	}

	public static void main(String[] args) {
		System.out.println("Hollo World!");

		MyLamda m1 = a -> a;
		
		System.out.println("m1:" + m1.add(1));
		//System.out.println("m2:" + m2.add(1,2));
		
	}

}