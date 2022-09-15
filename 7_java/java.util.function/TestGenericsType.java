import java.util.*;
import java.util.function.*;

class ArgObj<U, V, W/*Global type*/> {
	public U u;
	public V v;

	ArgObj(U a, V b){
		u = a;
		v = b;
	}
}

class MyGenerics<V, U, W/*Global type*/> {

	@SuppressWarnings("unchecked")
	public <A, B, R/*Local type*/> R collect(ArgObj<U, V, W> c){
		W w = null;
		A a = (A)c.u;
		B b = (B)c.v;
		R r = (R)new ArrayList();
		((Collection)r).add(a);
		((List)      r).add(b);
		((List)      r).add(w);
		return r;
	}
}

public class TestGenericsType {

	@SuppressWarnings("unchecked")
	public static void main(String[] args) {
		System.out.println("Hollo World!");
		
		ArgObj<String, Integer, List> arg = new ArgObj<String, Integer, List>("Hello", 123);
		
		MyGenerics<Integer, String, List> mg = new MyGenerics<Integer, String, List>();

		List list =(List) mg.<String, Integer, Collection>collect(arg);
		
		list.stream().forEach(e -> System.out.println(e));
	}
	
}