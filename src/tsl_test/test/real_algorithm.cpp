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

// Unlike other unit tests, which focus more on small part of the language supports.
// These unit tests are real algorithm that people use here and there in practice.
// Idealy, it should all run well.

#include "test_common.h"

// Factorial number generation
static int factorial_reference(int k) {
    if (k == 0)
        return 1;
    return k * factorial_reference(k - 1);
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

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int, int*)>(shader_source, shading_system);

    int test_value = 1;
    func_ptr(10, &test_value);
    EXPECT_EQ(test_value, factorial_reference(10));
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

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int, int*)>(shader_source, shading_system);

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

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int, int, int*)>(shader_source, shading_system);

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

// reference implementation
static int reverse_ref(int x) {
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

        shader ververse_func( int a , out int o0 ){
            o0 = reverse( a );
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int, int*)>(shader_source, shading_system);

    auto verify_func = [&](int a) {
        int o0;
        func_ptr(a, &o0);
        EXPECT_EQ(reverse_ref(a) , o0);
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

// reference implementation
bool isPalindrome_ref(int x) {
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
        int isPalindrome(int x) {
            if (x < 0)
                return 0;

            int rev = 0;
            int old = x;
            while (x != 0) {
                if (rev > 0xefffffff / 10 || (rev == 0xefffffff / 10 && rev > 7))
                    return 0;

                rev = (rev * 10) + (x % 10);
                x /= 10;
            }

            if (old == rev && rev >= 0)
                return 1;
            return 0;
        }

        shader ververse_func( int a , out int o0 ){
            o0 = isPalindrome( a );
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int, int*)>(shader_source, shading_system);

    auto verify_func = [&](int a) {
        int o0;
        func_ptr(a, &o0);
        EXPECT_EQ((int)isPalindrome_ref(a), o0);
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

// Reference implementation
float myPow_ref(float x, long long n) {
    if (n == 0)
        return 1.0;

    if (n > 0) {
        float half_power = myPow_ref(x, n / 2);
        return (n % 2) ? half_power * half_power * x : half_power * half_power;
    }
    return 1.0f / myPow_ref(x, -n);
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

        shader myPow_func( float x, int n , out float o0 ){
            o0 = myPow( x , n );
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(float, int, float*)>(shader_source, shading_system);

    auto verify_func = [&](float x, int a) {
        float o0;
        func_ptr(x, a, &o0);
        EXPECT_EQ(myPow_ref(x, a), o0);
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

// Reference implementation
int climbStairs_ref(int n) {
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
            int a = 1, b = 2, c = 3;
            for (int i = 3; i <= n; ++i){
                c = a + b;
                a = b;
                b = c;
            }
            return c;
        }

        shader myPow_func( int n , out int o0 ){
            o0 = climbStairs( n );
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int, int*)>(shader_source, shading_system);

    auto verify_func = [&](int x) {
        int o0;
        func_ptr(x, &o0);
        EXPECT_EQ(climbStairs_ref(x), o0);
    };

    verify_func(1);
    verify_func(123);
    verify_func(0);
    verify_func(121);
    verify_func(1024);
    verify_func(10);
}