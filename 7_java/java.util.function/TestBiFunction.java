import java.util.*;
import java.util.function.*;

public class TestBiFunction{

	public static void main(String[] args) {
		System.out.println("Hollo World!");

		BiFunction<Integer, Integer, Integer> bf1 = (a, b) -> a + b;
		System.out.println("result:" + bf1.apply(3, 4));

		BiFunction<String, Integer, String> bf2 = (a, b) -> a + b;
		System.out.println("result:" + bf2.apply("3", 4));

		BiFunction<Integer, Integer, String> bf3 = (a, b) -> a + b + "";
		System.out.println("result:" + bf3.apply(3, 4));

		BiFunction<Integer, Integer, String> bf4 = (a, b) -> "" + a + b;
		System.out.println("result:" + bf4.apply(3, 4));

	}

}
/*
result:7
result:34
result:7
result:34
*/