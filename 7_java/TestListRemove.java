
import java.util.*;

public class TestListRemove {
	
	public static void main(String[] args) {
		System.out.println("Hello World");
		ArrayList<Integer> list = new ArrayList<Integer>();
		
		for (int i =0; i < 10; i++) {
			list.add(new Integer(i));
		}
		
		for (int i =0; i < list.size(); i++) {
			Integer e = list.get(i);
			list.remove(e);
			//i--;
			System.out.println("remove:" + i + ",size:" + list.size());
		}
		
		for(Integer i: list) {
			System.out.println(i);
		}
	}
}
