import java.util.*;
import java.util.function.*;

public class TestPredicate {


	public static void main(String[] args) {
		System.out.println("Hollo World!");

		Predicate<String> p1 = t -> Integer.valueOf(t) > 0;
		System.out.println("Predicate:" + p1.test("1"));

		Predicate<Double> p = t -> t > 0.00000001;
		System.out.println("Predicate:" + p.test(0.1));
	}
	
}