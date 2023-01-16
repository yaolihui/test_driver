import java.lang.ref.SoftReference;
import java.lang.ref.WeakReference;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.PhantomReference;


public class TestReferrence {
	static SoftReference<String> stringSoftReference = new SoftReference<>(new String("软 引用 仔"));
	static WeakReference<String> str = new WeakReference<>("弱reference 森");
    static PhantomReference<String> strs = new PhantomReference<>("虚引用", new ReferenceQueue<>());

	public void testSoftReference() {
		//强引用
        String S = "强 引用 森";
        System.out.println(S);
		System.out.println();

        //软引用
        System.out.println(stringSoftReference.get());
        System.gc();
        System.out.println(stringSoftReference.get());
		System.out.println();

        //虚引用
        System.out.println(strs.get());
		System.gc();
        System.out.println(strs.get());
		System.out.println();
	}

	public void testWeakReference() {
        //弱引用
        System.out.println(str.get());
        System.gc();
        System.out.println(str.get());
		System.out.println();

	}

	public static void main(String[] arg) {
		System.out.println("enter!");
		
		TestReferrence tr = new TestReferrence();
		tr.testSoftReference();
		tr.testWeakReference();
		try {Thread.sleep(5000);} catch(InterruptedException e){}
		tr.testWeakReference();
	}
	
}