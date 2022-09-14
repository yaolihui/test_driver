import java.util.*;
import java.util.function.*;


public class TestConsumer {


	public static void main(String[] args) {
		System.out.println("Hollo World!");
		
		ArrayList<String> list = new ArrayList<String>();
		
		Consumer<ArrayList> c1 = new Consumer<ArrayList>(){ 
			public void accept(ArrayList t){
				t.add("111");
			}
		};		
		Consumer<ArrayList> c2 = t -> t.add("111");		
		Consumer<ArrayList> c3 = t -> System.out.println(t.size());
		
		System.out.println("------------------");
		
		c1.andThen(c2).andThen(c3).accept(list);
	}
	
}