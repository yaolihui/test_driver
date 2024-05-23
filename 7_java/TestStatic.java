class TestStaticInner {
	static int a = 0;
	String b = "b";
}

public class TestStatic {

	public static void main(String[] arg) {
		TestStaticInner inner = new TestStaticInner();
		System.out.println("inner.a=" + inner.a);
		System.out.println("inner.b=" + inner.b);
		System.out.println("======================");
		inner.a = 1;
		inner.b = "bb";
		TestStaticInner inner2 = new TestStaticInner();
		System.out.println("inner2.a=" + inner2.a);
		System.out.println("inner2.b=" + inner2.b);

		System.out.println("Hello world");
	}
}
