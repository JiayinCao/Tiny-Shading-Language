/*
    This file is a part of Tiny-Shading-Language or TSL, an open-source cross
    platform programming shading language.

    Copyright (c) 2020-2020 by Jiayin Cao - All rights reserved.

    TSL is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

// Though TSL is a shading language, the fact it supports recursively function calls
// means that I can use it to solve general purpose problems.
//
// Unlike other unit tests, which focus more on small part of the language supports.
// These unit tests are real algorithm that people use here and there in practice.
// Idealy, it should all run well.
//
// Lots of the problems come from Leetcode. N.B, the solutions provided here is not
// necessary the best solutions to the original problem, it only just solves the problem
// in a correct way. The coding style is intentionally different and a bit un-elegant
// so that it can test how robust the shading language is.

#include <vector>
#include "test_common.h"

// Factorial number generation
static int factorial(int k) {
    if (k == 0)
        return 1;
    return k * factorial(k - 1);
};
TEST(Practical, Factorial) {
    auto shader_source = R"(
        int factorial( int k ){
            if( !k )
                return 1;
            return k * factorial( k - 1 );
        }

        shader main(int arg0 = 0, out int arg2 = 5){
			arg2 = factorial( arg0 );
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    int test_value = 1;
    func_ptr(10, &test_value);
    EXPECT_EQ(test_value, factorial(10));
}


// Fibnoacci number
static int fibonacci(int k) {
    if (k <= 1) return k;
    return fibonacci(k - 1) + fibonacci(k - 2);
}
TEST(Practical, Fibonacci) {
    auto shader_source = R"(
        int fibonacci( int k ){
            if( k <= 1 ) return k;
            return fibonacci(k-1) + fibonacci(k-2);
        }
        
        shader main(int arg0, out int arg2){
			arg2 = fibonacci( arg0 );
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    int test_value = 1;
    func_ptr(10, &test_value);
    EXPECT_EQ(test_value, fibonacci(10));
}


// Sum of Two Integers
// https://leetcode.com/problems/sum-of-two-integers/
//
// Calculate the sum of two integers a and b, but you are not allowed to use the operator + and -.
TEST(Practical, Sum_of_Two_Integers) {
    auto shader_source = R"(
        int internal_helper( int a , int b ){
            int c = a & b;
            if( c == 0 )
                return a | b;
            return internal_helper( c << 1 , a ^ b );
        }

        shader sum_of_two_integers( int a , int b , out int o0 ){
            o0 = internal_helper( a , b );
        }
    )";

    auto ret = compile_shader<void(*)(int, int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int a, int b) {
        int o0;
        func_ptr(a, b, &o0);
        EXPECT_EQ(a + b, o0);
    };

    verify_func(1, 1);
    verify_func(23, 12);
    verify_func(0, 1024);
    verify_func(1, 213);

    verify_func(std::numeric_limits<int>::min(), 12);
    verify_func(std::numeric_limits<int>::max(), 12);
    verify_func(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
}


// Reverse Integer
// https://leetcode.com/problems/reverse-integer/
//
// Given a 32-bit signed integer, reverse digits of an integer.

static int reverse(int x) {
    int reverse = 0;
    while (x != 0)
    {
        if (214748364 < reverse || -214748364 > reverse)
            return 0;
        reverse = (reverse * 10) + (x % 10);
        x /= 10;
    }
    return reverse;
}

TEST(Practical, Reverse_Integer) {
    auto shader_source = R"(
        int reverse(int x) {
            int reverse =0;
            while (x!=0)
            {
                if( 214748364 < reverse || -214748364 > reverse)
                    return 0;
                reverse = (reverse * 10) + (x %10);
                x/=10;
            }
            return reverse;
        }

        shader main( int a , out int o0 ){
            o0 = reverse( a );
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int a) {
        int o0;
        func_ptr(a, &o0);
        EXPECT_EQ(reverse(a) , o0);
    };

    verify_func(1);
    verify_func(23);
    verify_func(0);
    verify_func(1231);
    verify_func(std::numeric_limits<int>::min());
    verify_func(std::numeric_limits<int>::max());
}


// Palindrome Number
// https://leetcode.com/problems/palindrome-number/
//
// Determine whether an integer is a palindrome. An integer is a palindrome when it reads the same backward as forward.

static bool isPalindrome(int x) {
    if (x < 0)
        return false;

    int rev = 0;
    int old = x;
    while (x != 0) {
        if (rev > std::numeric_limits<int>::max() / 10 || (rev == std::numeric_limits<int>::max() / 10 && rev > 7))
            return false;

        rev = (rev * 10) + (x % 10);
        x /= 10;
    }

    if (old == rev && rev >= 0)
        return true;
    return false;
}

TEST(Practical, Is_Palindrome) {
    auto shader_source = R"(
        bool isPalindrome(int x) {
            if (x < 0)
                return false;

            int rev = 0;
            int old = x;
            while (x != 0) {
                if (rev > 0xefffffff / 10 || (rev == 0xefffffff / 10 && rev > 7))
                    return false;

                rev = (rev * 10) + (x % 10);
                x /= 10;
            }

            if (old == rev && rev >= 0)
                return true;
            return false;
        }

        shader main( int a , out bool o0 ){
            o0 = isPalindrome( a );
        }
    )";

    auto ret = compile_shader<void(*)(int, bool*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int a) {
        bool o0;
        func_ptr(a, &o0);
        EXPECT_EQ(isPalindrome(a), o0);
    };

    verify_func(1);
    verify_func(123);
    verify_func(0);
    verify_func(121);
    verify_func(std::numeric_limits<int>::min());
    verify_func(std::numeric_limits<int>::max());
}

// Pow(x, n)
// https://leetcode.com/problems/powx-n/submissions/
//
// Implement pow(x, n), which calculates x raised to the power n, (x^n).

static float myPow(float x, long long n) {
    if (n == 0)
        return 1.0;

    if (n > 0) {
        float half_power = myPow(x, n / 2);
        return (n % 2) ? half_power * half_power * x : half_power * half_power;
    }
    return 1.0f / myPow(x, -n);
}

TEST(Practical, myPow) {
    auto shader_source = R"(
        float myPow(float x, int n) {
            if (n == 0)
                return 1.0;

            if (n > 0) {
                float half_power = myPow(x, n / 2);
                return (n % 2) ? half_power * half_power * x : half_power * half_power;
            }
            return 1.0 / myPow(x, -n);
        }

        shader main( float x, int n , out float o0 ){
            o0 = myPow( x , n );
        }
    )";

    auto ret = compile_shader<void(*)(float, int, float*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](float x, int a) {
        float o0;
        func_ptr(x, a, &o0);
        EXPECT_EQ(myPow(x, a), o0);
    };

    verify_func(1.0f, 1);
    verify_func(2.0f, 123);
    verify_func(3.0f, 0);
    verify_func(4.0f, -121);
    verify_func(1024.0f, -1);
    verify_func(1024.0f, -10);
}

// Climbing Stairs
// https://leetcode.com/problems/climbing-stairs/
//
// You are climbing a stair case. It takes n steps to reach to the top.
// Each time you can either climb 1 or 2 steps.In how many distinct ways can you climb to the top ?

static int climbStairs(int n) {
    if (n == 1) return 1;
    if (n == 2) return 2;
    int a = 1, b = 2, c = 3;
    for (int i = 3; i <= n; ++i)
    {
        c = a + b;
        a = b;
        b = c;
    }
    return c;
}

TEST(Practical, ClimbingStairs) {
    auto shader_source = R"(
        int climbStairs(int n) {
            if (n == 1) return 1;
            if (n == 2) return 2;
            int a = 1;
            int b = 2;
            int c = 3;
            for (int i = 3; i <= n; ++i){
                c = a + b;
                a = b;
                b = c;
            }
            return c;
        }

        shader main( int n , out int o0 ){
            o0 = climbStairs( n );
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x) {
        int o0;
        func_ptr(x, &o0);
        EXPECT_EQ(climbStairs(x), o0);
    };

    verify_func(1);
    verify_func(123);
    verify_func(0);
    verify_func(121);
    verify_func(1024);
    verify_func(10);
}


// Bitwise AND of Numbers Range
// https://leetcode.com/problems/bitwise-and-of-numbers-range/
//
// Given a range [m, n] where 0 <= m <= n <= 2147483647, return the bitwise AND of all numbers in this range, inclusive.

static int rangeBitwiseAnd(int m, int n) {
    if (m == n)
        return m;
    return rangeBitwiseAnd(m >> 1, n >> 1) << 1;
}

TEST(Practical, RangeBitwiseAnd) {
    auto shader_source = R"(
        int rangeBitwiseAnd(int m, int n) {
            if (m == n)
                return m;
            return rangeBitwiseAnd(m >> 1, n >> 1) << 1;
        }

        shader main( int m , int n , out int o0 ){
            o0 = rangeBitwiseAnd( m , n );
        }
    )";

    auto ret = compile_shader<void(*)(int, int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int m, int n) {
        int o0;
        func_ptr(m, n, &o0);
        EXPECT_EQ(rangeBitwiseAnd(m, n), o0);
    };

    verify_func(1, 0);
    verify_func(123, 123);
    verify_func(0, 11);
    verify_func(121, 1024);
    verify_func(1024, 65536);
    verify_func(10, std::numeric_limits<int>::max());
}


// Happy Number
// https://leetcode.com/problems/happy-number/
//
// Write an algorithm to determine if a number n is "happy".
// A happy number is a number defined by the following process: Starting with any positive integer, 
// replace the number by the sum of the squares of its digits, and repeat the process until the number 
// equals 1 (where it will stay), or it loops endlessly in a cycle which does not include 1. Those 
// numbers for which this process ends in 1 are happy numbers.
// Return True if n is a happy number, and False if not.

static int next(int n) {
    int ret = 0;
    while (n) {
        int k = n % 10;
        n /= 10;
        ret += k * k;
    }
    return ret;
}

static bool isHappy(int n) {
    int slow = n, fast = n;
    do {
        slow = next(slow);
        fast = next(next(fast));

        if (fast == 1)
            return true;

    } while (slow != fast);

    return false;
}

TEST(Practical, HappyNumber) {
    auto shader_source = R"(
        int next(int n) {
            int ret = 0;
            while (n) {
                int k = n % 10;
                n /= 10;
                ret += k * k;
            }
            return ret;
        }

        bool isHappy(int n) {
            int slow = n;
            int fast = n;
            do {
                slow = next(slow);
                fast = next(next(fast));

                if (fast == 1)
                    return true;

            } while (slow != fast);

            return false;
        }

        shader main( int m , out bool o0 ){
            o0 = isHappy( m );
        }
    )";

    auto ret = compile_shader<void(*)(int, bool*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int m) {
        bool o0;
        func_ptr(m, &o0);
        EXPECT_EQ(isHappy(m), o0);
    };

    verify_func(1);
    verify_func(123);
    verify_func(0);
    verify_func(121);
    verify_func(1024);
    verify_func(std::numeric_limits<int>::max());
    verify_func(std::numeric_limits<int>::min());
}

// Number of Digit One
// https://leetcode.com/problems/number-of-digit-one/
//
// Given an integer n, count the total number of digit 1 appearing in all non-negative integers less than or equal to n.

static int countDigitOne(int n) {
    if (n <= 0) {
        return 0;
    }
    int m = n;
    int sum = 0;
    int e = 1;
    while (n > 0) {
        int r = n % 10;
        n /= 10;
        if (r == 0)
            sum += n * e;
        else if (r > 1)
            sum += (n + 1) * e;
        else
            sum += m - n * 9 * e - e + 1;
        if (n > 0)
            e *= 10;
    }
    return sum;
}

TEST(Practical, CountDigitOne) {
    auto shader_source = R"(
        int countDigitOne(int n) {
            if (n <= 0) {
                return 0;
            }
            int m = n;
            int sum = 0;
            int e = 1;
            while (n > 0) {
                int r = n % 10;
                n /= 10;
                if (r == 0)
                    sum += n * e;
                else if (r > 1)
                    sum += (n + 1) * e;
                else
                    sum += m - n * 9 * e - e + 1;
                if (n > 0)
                    e *= 10;
            }
            return sum;
        }

        shader main( int m , out int o0 ){
            o0 = countDigitOne( m );
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int m) {
        int o0;
        func_ptr(m, &o0);
        EXPECT_EQ(countDigitOne(m), o0);
    };

    verify_func(1);
    verify_func(123);
    verify_func(0);
    verify_func(121);
    verify_func(1024);
    verify_func(std::numeric_limits<int>::max());
    verify_func(std::numeric_limits<int>::min());
}


// Power of Two
// https://leetcode.com/problems/power-of-two/
//
// Given an integer, write a function to determine if it is a power of two.

static bool isPowerOfTwo(int n) {
    return (n <= 0) ? false : !(n & (n - 1));
}

TEST(Practical, PowerOfTwo) {
    auto shader_source = R"(
        bool isPowerOfTwo(int n) {
            return (n <= 0) ? false : !(n & (n - 1));
        }

        shader main( int m , out bool o0 ){
            o0 = isPowerOfTwo( m );
        }
    )";

    auto ret = compile_shader<void(*)(int, bool*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int m) {
        bool o0;
        func_ptr(m, &o0);
        EXPECT_EQ(isPowerOfTwo(m), o0);
    };

    verify_func(1);
    verify_func(123);
    verify_func(0);
    verify_func(-12);
    verify_func(121);
    verify_func(1024);
    verify_func(std::numeric_limits<int>::max());
    verify_func(std::numeric_limits<int>::min());
}


// Add Digits
// https://leetcode.com/problems/add-digits/
//
// Given a non-negative integer num, repeatedly add all its digits until the result has only one digit.

static int addDigits(int num) {
    if (num == 0)
        return 0;

    int k = num % 9;
    return k ? k : 9;
}

TEST(Practical, AddDigits) {
    auto shader_source = R"(
        int addDigits(int num) {
            if (num == 0)
                return 0;

            int k = num % 9;
            return k ? k : 9;
        }

        shader main( int m , out int o0 ){
            o0 = addDigits( m );
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int m) {
        int o0;
        func_ptr(m, &o0);
        EXPECT_EQ(addDigits(m), o0);
    };

    verify_func(1);
    verify_func(123);
    verify_func(0);
    verify_func(-12);
    verify_func(121);
    verify_func(1024);
    verify_func(std::numeric_limits<int>::max());
    verify_func(std::numeric_limits<int>::min());
}


// Ugly Number
// https://leetcode.com/problems/ugly-number/
//
// Write a program to check whether a given number is an ugly number.
// Ugly numbers are positive numbers whose prime factors only include 2, 3, 5.

static bool isUgly(int num) {
    if (num == 0) return false;
    while (num % 2 == 0) num /= 2;
    while (num % 3 == 0) num /= 3;
    while (num % 5 == 0) num /= 5;
    return num == 1;
}

TEST(Practical, IsUglyNumber) {
    auto shader_source = R"(
        bool isUgly(int num) {
            if (num == 0) return false;
            while (num % 2 == 0) num /= 2;
            while (num % 3 == 0) num /= 3;
            while (num % 5 == 0) num /= 5;
            return num == 1;
        }

        shader main( int m , out bool o0 ){
            o0 = isUgly( m );
        }
    )";

    auto ret = compile_shader<void(*)(int, bool*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int m) {
        bool o0;
        func_ptr(m, &o0);
        EXPECT_EQ(isUgly(m), o0);
    };

    verify_func(1);
    verify_func(123);
    verify_func(0);
    verify_func(-12);
    verify_func(121);
    verify_func(1024);
    verify_func(std::numeric_limits<int>::max());
    verify_func(std::numeric_limits<int>::min());
}


// Nim Game
// https://leetcode.com/problems/nim-game/
//
// You are playing the following Nim Game with your friend: There is a heap of stones on the table, 
// each time one of you take turns to remove 1 to 3 stones. The one who removes the last stone will
// be the winner. You will take the first turn to remove the stones.

static int canWinNim(int n) {
    return n % 4;
}

TEST(Practical, WinNim) {
    auto shader_source = R"(
        int canWinNim(int n) {
            return n % 4;
        }

        shader main( int m , out int o0 ){
            o0 = canWinNim( m );
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int m) {
        int o0;
        func_ptr(m, &o0);
        EXPECT_EQ(canWinNim(m), o0);
    };

    verify_func(1);
    verify_func(123);
    verify_func(0);
    verify_func(-12);
    verify_func(121);
    verify_func(1024);
    verify_func(std::numeric_limits<int>::max());
    verify_func(std::numeric_limits<int>::min());
}


// Power of Three
// https://leetcode.com/problems/power-of-three/

static bool isPowerOfThree(int n) {
    if (n < 1) return false;
    if (n == 1) return true;
    if (n > 1 && 1162261467 % n == 0)
        return true;
    return false;
}

TEST(Practical, PowerOfThree) {
    auto shader_source = R"(
        bool isPowerOfThree(int n) {
            if (n < 1) return false;
            if (n == 1) return true;
            if (n > 1 && 1162261467 % n == 0)
                return true;
            return false;
        }

        shader main( int m , out bool o0 ){
            o0 = isPowerOfThree( m );
        }
    )";

    auto ret = compile_shader<void(*)(int, bool*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int m) {
        bool o0;
        func_ptr(m, &o0);
        EXPECT_EQ(isPowerOfThree(m), o0);
    };

    verify_func(1);
    verify_func(123);
    verify_func(0);
    verify_func(-12);
    verify_func(121);
    verify_func(1024);
    verify_func(std::numeric_limits<int>::max());
    verify_func(std::numeric_limits<int>::min());
}


// Water and Jug Problem
// https://leetcode.com/problems/water-and-jug-problem/
//
// You are given two jugs with capacities x and y litres. There is an infinite amount of water supply available. 
// You need to determine whether it is possible to measure exactly z litres using these two jugs.
// If z liters of water is measurable, you must have z liters of water contained within one or both buckets by the end.
// Operations allowed:
//  - Fill any of the jugs completely with water.
//  - Empty any of the jugs.
//  - Pour water from one jug into another till the other jug is completely full or the first jug itself is empty.

static int gcd(int a, int b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

static bool canMeasureWater(int x, int y, int z) {
    if (z == 0)
        return true;

    if (x + y >= z) {
        if (z % gcd(x, y) == 0)
            return true;
    }

    return false;
}

TEST(Practical, CanMeasureWater) {
    auto shader_source = R"(
        int gcd(int a, int b) {
            if (b == 0)
                return a;
            return gcd(b, a % b);
        }

        bool canMeasureWater(int x, int y, int z) {
            if (z == 0)
                return true;

            if(x + y >= z){
                if(z % gcd(x, y) == 0)
                    return true;
            }

            return false;
        }

        shader main( int _x, int _y, int _z, out bool o0 ){
            o0 = canMeasureWater( _x, _y, _z );
        }
    )";

    auto ret = compile_shader<void(*)(int, int, int, bool*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x, int y, int z) {
        bool o0;
        func_ptr(x, y, z, &o0);
        EXPECT_EQ(canMeasureWater(x,y,z), o0);
    };

    verify_func(0,0,0);
    verify_func(1,4,2);
    verify_func(1,134,1024);
    verify_func(23, 53, 512);
    verify_func(12, 23131, 1231123);
    verify_func(21343, 1231, 1231231);
    verify_func(123, 123123, std::numeric_limits<int>::max());
}

// Valid Perfect Square
// https://leetcode.com/problems/valid-perfect-square/
//
// Given a positive integer num, write a function which returns True if num is a perfect square else False.
// Unlike the original problem, overflow is not taken care of.

static bool isPerfectSquare(int n) {
    if (n <= 0) return false;
    if (n == 1) return true;
    for (int i = 2; i <= n / 2; i++) {
        int k = i * i;
        if (k == n) {
            return true;
            break;
        }
        else if (k > n)
            return false;
    }
    return false;
}

TEST(Practical, ValidPerfectSquare) {
    auto shader_source = R"(
        bool isPerfectSquare(int n) {
            if (n <= 0) return false;
            if (n == 1) return true;
            for (int i = 2; i <= n / 2; i++) {
                int k = i * i;
                if (k == n) {
                    return true;
                }
                else if (k > n)
                    return false;
            }
            return false;
        }

        shader main( int m, out bool o0 ){
            o0 = isPerfectSquare(m);
        }
    )";

    auto ret = compile_shader<void(*)(int, bool*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x) {
        bool o0;
        func_ptr(x, &o0);
        EXPECT_EQ(isPerfectSquare(x), o0);
    };

    verify_func(0);
    verify_func(1);
    verify_func(1123);
    verify_func(23);
    verify_func(64);
    verify_func(1024);
}

// Elimination Game
// https://leetcode.com/problems/elimination-game/
//
// There is a list of sorted integers from 1 to n. Starting from left to right, remove the first number and 
// every other number afterward until you reach the end of the list. Repeat the previous step again, but this 
// time from right to left, remove the right most number and every other number from the remaining numbers.
// We keep repeating the steps again, alternating left to right and right to left, until a single number remains.
// Find the last number that remains starting with a list of length n.

static int lastRemaining(int n) {
    if (n == 1)
        return 1;
    return 2 * (1 + n / 2 - lastRemaining(n / 2));
}

TEST(Practical, LastRemaining) {
    auto shader_source = R"(
        int lastRemaining(int n) {
            if (n == 1)
                return 1;
            return 2 * (1 + n / 2 - lastRemaining(n / 2));
        }

        shader main( int m, out int o0 ){
            o0 = lastRemaining(m);
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x) {
        int o0;
        func_ptr(x, &o0);
        EXPECT_EQ(lastRemaining(x), o0);
    };

    verify_func(1);
    verify_func(12);
    verify_func(23);
    verify_func(64);
    verify_func(1024);
}

// Integer Replacement
// https://leetcode.com/problems/integer-replacement/
//
// Given a positive integer n and you can do operations as follow:
//  - If n is even, replace n with n/2.
//  - If n is odd, you can replace n with either n + 1 or n - 1.
// What is the minimum number of replacements needed for n to become 1?

static int integerReplacement(int n) {
    if (n <= 1) return 0;
    if (n == 2147483647)
        return 32;

    if (n % 2 == 0)
        return 1 + integerReplacement(n / 2);
    
    int a = integerReplacement(n + 1) + 1;
    int b = integerReplacement(n - 1) + 1;

    return a < b ? a : b;
}

TEST(Practical, Integer_Replacement) {
    auto shader_source = R"(
        int integerReplacement(int n) {
            if (n <= 1) return 0;
            if (n == 2147483647)
                return 32;

            if (n % 2 == 0)
                return 1 + integerReplacement(n / 2);
    
            int a = integerReplacement(n + 1) + 1;
            int b = integerReplacement(n - 1) + 1;

            return a < b ? a : b;
        }

        shader main( int m, out int o0 ){
            o0 = integerReplacement(m);
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x) {
        int o0;
        func_ptr(x, &o0);
        EXPECT_EQ(integerReplacement(x), o0);
    };

    verify_func(1);
    verify_func(12);
    verify_func(23);
    verify_func(64);
    verify_func(1024);
    verify_func(std::numeric_limits<int>::max());
}

// Nth Digit
// https://leetcode.com/problems/nth-digit/
//
// Find the nth digit of the infinite integer sequence 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, ...

// this doesn't handle negative 'n'
static int mypow(int x, int n) {
    if (n == 0)
        return 1;

    int half_power = mypow(x, n / 2);
    return (n % 2) ? half_power * half_power * x : half_power * half_power;
}

static int findNthDigit(int n) {
    int i = 1;
    int curNumLenCount = 1;
    int curCount = 0;
    curCount = i * 9 * curNumLenCount;
    while (n > curCount){
        n -= curCount;
        curNumLenCount *= 10;
        i++;
        curCount = i * 9 * curNumLenCount;
    }

    if (n % i == 0)
        return (curNumLenCount + n / i - 1) % 10;

    return ((curNumLenCount + n / i) / int(mypow(10, (i - n % i)))) % 10;
}

TEST(Practical, FindNthDigit) {
    auto shader_source = R"(
        int mypow(int x, int n) {
            if (n == 0)
                return 1;

            int half_power = mypow(x, n / 2);
            return (n % 2) ? half_power * half_power * x : half_power * half_power;
        }

        int findNthDigit(int n) {
            int i = 1;
            int curNumLenCount = 1;
            int curCount = 0;
            curCount = i * 9 * curNumLenCount;
            while (n > curCount){
                n -= curCount;
                curNumLenCount *= 10;
                i++;
                curCount = i * 9 * curNumLenCount;
            }

            if (n % i == 0)
                return (curNumLenCount + n / i - 1) % 10;

            return ((curNumLenCount + n / i) / mypow(10, (i - n % i))) % 10;
        }

        shader main( int m, out int o0 ){
            o0 = findNthDigit(m);
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x) {
        int o0;
        func_ptr(x, &o0);
        EXPECT_EQ(findNthDigit(x), o0);
    };

    verify_func(1);
    verify_func(12);
    verify_func(23);
    verify_func(64);
    verify_func(1024);
}

// K-th Smallest in Lexicographical Order
// Given integers n and k, find the lexicographically k-th smallest integer in the range from 1 to n.
// Note: 1 <= k <= n <= 10^9.

static int findKthNumber(int n, int k) {
    int cnt = 1;
    k--;
    while (k) {
        int st = 0, head = cnt, tail = cnt + 1;
        while (head <= n) {
            if (tail > n + 1)
                st += n + 1 - head;
            else
                st += tail - head;
            head *= 10;
            tail *= 10;
        }

        if (st <= k) {
            cnt++;
            k -= st;
        }
        else {
            cnt *= 10;
            k--;
        }
    }
    return cnt;
}

TEST(Practical, FindKthNumber) {
    auto shader_source = R"(
        int findKthNumber(int n, int k) {
            int cnt = 1;
            k--;
            while (k) {
                int st = 0;
                int head = cnt;
                int tail = cnt + 1;
                while (head <= n) {
                    if (tail > n + 1)
                        st += n + 1 - head;
                    else
                        st += tail - head;
                    head *= 10;
                    tail *= 10;
                }

                if (st <= k) {
                    cnt++;
                    k -= st;
                }
                else {
                    cnt *= 10;
                    k--;
                }
            }
            return cnt;
        }

        shader main( int n, int k, out int o0 ){
            o0 = findKthNumber(n, k);
        }
    )";

    auto ret = compile_shader<void(*)(int, int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int n, int k) {
        int o0;
        func_ptr(n, k, &o0);
        EXPECT_EQ(findKthNumber(n,k), o0);
    };

    for (int i = 124; i < 1024; ++i)
        verify_func(i, 123);

    verify_func(13, 2);
    verify_func(1024, 3);
    verify_func(21, 3);
    verify_func(32, 21);
    verify_func(32134, 123);
}


// Poor Pigs
// https://leetcode.com/problems/poor-pigs/
//
// There are 1000 buckets, one and only one of them is poisonous, while the rest are filled with water. 
// They all look identical. If a pig drinks the poison it will die within 15 minutes. What is the minimum 
// amount of pigs you need to figure out which bucket is poisonous within one hour?

static int poorPigs(int buckets, int minutesToDie, int minutesToTest) {
    int numIntervals = minutesToTest / minutesToDie + 1;

    if (buckets == 1)
        return 0;

    int numStatesPossible = numIntervals;
    int pig = 1;
    while (pig < 32 && numStatesPossible < buckets) {
        numStatesPossible = numStatesPossible * numIntervals;
        pig++;
    }
    return pig;
}

TEST(Practical, PoorPigs) {
    auto shader_source = R"(
        int poorPigs(int buckets, int minutesToDie, int minutesToTest) {
            int numIntervals = minutesToTest / minutesToDie + 1;

            if (buckets == 1)
                return 0;

            int numStatesPossible = numIntervals;
            int pig = 1;
            while (pig < 32 && numStatesPossible < buckets) {
                numStatesPossible = numStatesPossible * numIntervals;
                pig++;
            }
            return pig;
        }

        shader main( int a, int b, int c, out int o0 ){
            o0 = poorPigs(a, b, c);
        }
    )";

    auto ret = compile_shader<void(*)(int, int, int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int a, int b, int c) {
        int o0;
        func_ptr(a, b, c, &o0);
        EXPECT_EQ(poorPigs(a, b, c), o0);
    };

    verify_func(1000, 15, 16);
    verify_func(1000, 15, 60);
    verify_func(4, 15, 15);
    verify_func(123, 15, 23);
    verify_func(32421, 10, 123);
}

// Non-negative Integers without Consecutive Ones
// https://leetcode.com/problems/non-negative-integers-without-consecutive-ones/
//
// Given a positive integer n, find the number of non-negative integers less than or equal to n, whose binary representations do NOT contain consecutive ones.

static void fun(int i, int& n, int& s) {
    if (i <= n) {
        if (i & 1) {
            s += 1;
            i = i << 1;
            fun(i, n, s);
        }
        else {
            if (i & 2) {
                s += 1;
                i = i << 1;
                fun(i, n, s);
            }
            else {
                s += 1;
                if (i + 1 <= n)
                    s += 1;
                int t = i + 1;
                i = i << 1;
                fun(i, n, s);
                if (t <= n) {
                    t = t << 1;
                    fun(t, n, s);
                }
            }
        }
    }
    return;
}
static int findIntegers(int num) {
    int s = 1;
    if (num) {
        int n = num;
        fun(1, n, s);
        return s;
    }
    return s;
}

TEST(Practical, FindIntegers) {
    auto shader_source = R"(
        void fun(int i, out int n, out int s) {
            if (i <= n) {
                if (i & 1) {
                    s += 1;
                    i = i << 1;
                    fun(i, n, s);
                }
                else {
                    if (i & 2) {
                        s += 1;
                        i = i << 1;
                        fun(i, n, s);
                    }
                    else {
                        s += 1;
                        if (i + 1 <= n)
                            s += 1;
                        int t = i + 1;
                        i = i << 1;
                        fun(i, n, s);
                        if (t <= n) {
                            t = t << 1;
                            fun(t, n, s);
                        }
                    }
                }
            }
            return;
        }
        int findIntegers(int num) {
            int s = 1;
            if (num) {
                int n = num;
                fun(1, n, s);
                return s;
            }
            return s;
        }

        shader main( int m, out int o0 ){
            o0 = findIntegers(m);
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x) {
        int o0;
        func_ptr(x, &o0);
        EXPECT_EQ(findIntegers(x), o0);
    };

    for( int i = 0 ; i < 1024 ; ++i )
        verify_func(i);
}

// Count Primes
// https://leetcode.com/problems/count-primes/
//
// Count the number of prime numbers less than a non-negative number, n.

static int countPrimes(int n) {
    if (n < 2) return 0;

    std::vector<int> a(n, 0);
    int count = 0;

    for (int i = 2; i < n; ++i) {
        if (a[i] == 0) {
            count++;
            for (int j = 1; j * i < n; ++j) {
                a[i * j] = 1;
            }
        }
    }
    return count;
}

TEST(Practical, CountPrimes) {
    auto shader_source = R"(
        int countPrimes(int n) {
            if(n<2) return 0;
        
            int a[n];
            for( int k = 0 ; k < n ; ++k )
                a[k] = 0;
        
            int count = 0;
            for(int i = 2; i<n; ++i){
                if(a[i] == 0){
                    count++;
                    for(int j = 1; j*i<n; ++j){
                        a[i*j] = 1;
                    }
                }
            }
            return count;
        }

        shader main( int m, out int o0 ){
            o0 = countPrimes(m);
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x) {
        int o0;
        func_ptr(x, &o0);
        EXPECT_EQ(countPrimes(x), o0);
    };

    for (int i = 0; i < 1024; ++i)
        verify_func(i);
}

// Ugly Number II
// https://leetcode.com/problems/ugly-number-ii/
//
// Write a program to find the n-th ugly number.
// Ugly numbers are positive numbers whose prime factors only include 2, 3, 5.

static int min(int x, int y) {
    return x < y ? x : y;
}

static int nthUglyNumber(int n) {
    int i2 = 1, i3 = 1, i5 = 1;

    std::vector<int> ugly(n+1);
    ugly[1] = 1;

    for (int i = 2; i <= n; i++) {
        ugly[i] = min(ugly[i2] * 2, min(ugly[i3] * 3, ugly[i5] * 5));
        if (ugly[i] == ugly[i2] * 2)
            i2++;
        if (ugly[i] == ugly[i3] * 3)
            i3++;
        if (ugly[i] == ugly[i5] * 5)
            i5++;
    }
    return ugly[n];
}

TEST(Practical, NthUglyNumber) {
    auto shader_source = R"(
        int min( int x , int y ){
            return x < y ? x: y;
        }

        int nthUglyNumber(int n) {
            int i2 = 1;
            int i3 = 1;
            int i5 = 1;

            int ugly[n + 1];
            ugly[1] = 1;

            for (int i = 2; i <= n; i++) {
                ugly[i] = min(ugly[i2] * 2, min(ugly[i3] * 3, ugly[i5] * 5));
                if (ugly[i] == ugly[i2] * 2)
                    i2++;
                if (ugly[i] == ugly[i3] * 3)
                    i3++;
                if (ugly[i] == ugly[i5] * 5)
                    i5++;
            }

            return ugly[n];
        }

        shader main( int m, out int o0 ){
            o0 = nthUglyNumber(m);
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x) {
        int o0;
        func_ptr(x, &o0);
        EXPECT_EQ(nthUglyNumber(x), o0);
    };

    for (int i = 1; i < 1024; ++i)
        verify_func(i);
}

// Perfect Squares
// https://leetcode.com/problems/perfect-squares/
//
// Given a positive integer n, find the least number of perfect square numbers (for example, 1, 4, 9, 16, ...) which sum to n.

static int numSquares(int n) {
    std::vector<int> dp(n + 1);
    dp[0] = 0;
    dp[1] = 1;
    for (int i = 2; i <= n; i++) {
        dp[i] = std::numeric_limits<int>::max();
        for (int j = 1; j * j <= i; j++) {
            dp[i] = min(dp[i], dp[i - j * j] + 1);
        }
    }
    return dp[n];
}

TEST(Practical, NumSquares) {
    auto shader_source = R"(
        int min( int a , int b ){
            return a < b ? a : b;
        }

        int numSquares(int n) {
            int dp[n + 1];
            dp[0] = 0;
            dp[1] = 1;
            for (int i = 2; i <= n; i++) {
                dp[i] = 0xefffffff;
                for (int j = 1; j * j <= i; j++) {
                    dp[i] = min(dp[i], dp[i - j * j] + 1);
                }
            }
            return dp[n];
        }

        shader main( int m, out int o0 ){
            o0 = numSquares(m);
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x) {
        int o0;
        func_ptr(x, &o0);
        EXPECT_EQ(numSquares(x), o0);
    };

    for (int i = 1; i < 1024; ++i)
        verify_func(i);
}

// Number Complement
// https://leetcode.com/problems/number-complement/
//
// Given a positive integer num, output its complement number. The complement strategy is to flip the bits of its binary representation.

int findComplement(int n) {
    int sum = 0, i = 0;
    while (n) {
        if (!(n & 1)) {
            sum += mypow(2, i);
        }
        n >>= 1;
        i++;
    }
    return sum;
}

TEST(Practical, FindComplement) {
    auto shader_source = R"(
        int mypow(int x, int n) {
            if (n == 0)
                return 1;

            int half_power = mypow(x, n / 2);
            return (n % 2) ? half_power * half_power * x : half_power * half_power;
        }

        int findComplement(int n) {
            int sum = 0;
            int i = 0;
            while (n) {
                if (!(n & 1)) {
                    sum += mypow(2, i);
                }
                n >>= 1;
                i++;
            }
            return sum;
        }

        shader main( int m, out int o0 ){
            o0 = findComplement(m);
        }
    )";

    auto ret = compile_shader<void(*)(int, int*)>(shader_source);
    auto func_ptr = ret.first;

    auto verify_func = [&](int x) {
        int o0;
        func_ptr(x, &o0);
        EXPECT_EQ(findComplement(x), o0);
    };

    for (int i = 1; i < 1024; ++i)
        verify_func(i);
}