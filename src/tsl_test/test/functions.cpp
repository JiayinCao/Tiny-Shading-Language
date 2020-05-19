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

#include "test_common.h"

TEST(Functions, Default_Shader) {
    validate_shader(R"(
        shader func(){
        }
    )");
}

TEST(Functions, Non_Shader) {
    validate_shader(R"(
        void none_shader_func(){}
    )");
}

TEST(Functions, Mixed_Shader) {
    validate_shader(R"(
        int none_shader_func111(){
            return 3;
        }
        
        shader shader_func()
        {
            {}
        }

        float non_shader_func222(){
            return 1.0;
        }
    )");
}

TEST(Functions, Single_Argument) {
    validate_shader(R"(
        int none_shader_func( int k )
        {
        }
    )");
}

TEST(Functions, Multi_Arguments) {
    validate_shader(R"(
        int none_shader_func( int arg0 , float arg1 , int arg2 , int arg3 )
        {
        }
    )");
}

TEST(Functions, Single_Argument_With_Defaults) {
    validate_shader(R"(
        int none_shader_func( float arg0 = 0.0 )
        {
        }
    )");
}

TEST(Functions, Multi_Argument_With_Defaults) {
    validate_shader(R"(
        // unlike C, there is no function overloading in TSL
        // default value can go to any argument, instead of just the last ones
        void none_shader_func( float arg0 = 0.0 , float arg1 , int arg2 = 0.0 , int arg3 )
        {
        }
    )");
}

TEST(Functions, Multi_Argument_With_Defaults_Multi_Line) {
    validate_shader(R"(
        // unlike C, there is no function overloading in TSL
        // default value can go to any argument, instead of just the last ones
        void none_shader_func( float arg0 = 0.0 , 
                          float arg1 , 
                          int arg2 = 0.0 , 
                          int arg3 ){
        }
    )");
}

TEST(Functions, Shader_Single_Argument) {
    validate_shader(R"(
        shader shader_func( float arg0 ){
            return;
        }
    )");
}

TEST(Functions, Shader_Single_Argument_With_Metadata ) {
    validate_shader(R"(
        shader shader_func( float arg0 <<< >>> ){
        }
    )");
}

TEST(Functions, Shader_Single_Argument_With_Metadata_and_Default ) {
    validate_shader(R"(
        shader shader_func( float arg0 = 0.0 <<< >>> ){
        }
    )");
}

TEST(Functions, Shader_Multi_Arguments_With_Metadata_and_Default ) {
    validate_shader(R"(
        shader shader_func( float arg0 = 0.0 <<< >>>,
                            int	  arg1 = 0  <<< >>> ,
                            int   arg2 = 2  <<<>>> ,
                            int   arg3 = 1  ){
        }
    )");
}

TEST(Functions, Non_Shader_Func_With_Return ) {
    validate_shader(R"(
        void generic_func( float arg0 = 0.0 ){
        }

        int generic_func2( float arg0 = 0.0 ){
            return a + generic_func( arg0 );
        }
    )");
}

TEST(Functions, Call_Function_No_Arg ) {
    validate_shader(R"(
        void generic_func( float arg0 = 0.0 ){
        }

        int generic_func2( float arg0 = 0.0 ){
            generic_func();
        
            return 2 + 12;
        }
    )");
}

TEST(Functions, Call_Function_Single_Arg ) {
    validate_shader(R"(
        void generic_func( float arg0 = 0.0 , int k = 0.0 ){
        }

        int generic_func2( float arg0 = 0.0 ){
            int arg0 = 0;

            // fix me
            generic_func( arg0 = 0 );

            return generic_func2();
        }
    )");
}

TEST(Functions, Call_Function_Multi_Args ) {
    validate_shader(R"(
        int generic_func2(){
            int k = 0;

            generic_func( arg0 , gg = 0 );

            return k = 2;
        }
    )");
}

TEST(Functions, Function_As_Argument ) {
    validate_shader(R"(
        int generic_func2(){
            int k = 0;

            generic_func( func( arg0 = 0 , arg1 = 0 ) , k );

            return 2;
        }
    )");
}

TEST(Functions, CONFIG_DECORATOR ) {
	validate_shader(R"(
        shader main( int arg0 = 232 ,
				     const int arg1 = 2,
					 in const float arg2 = 3.0,
					 const in matrix mat,
                     in float input_arg = 0.2 <<< >>> ,
                     out float last_arg = 2.0 <<< >>> ){
            arg2 = input_arg + 2.0;

            return arg0 + arg1;
        }
    )");
}

TEST(Functions, Simple_Test) {
	validate_shader(R"(
        shader main( out float arg2 ){
			int k = 0;
			
			while( k++ < 100 )
			{
			}

            arg2 = k == 100 ? 122.0 : 22.0;
        }
    )");
}

TEST(Functions, Single_Return) {
    validate_shader(R"(
        shader main(int arg0 = 0,
                    int arg1 = 2,
                    int arg2 = 3,
                    int arg3 = 4,
                    int arg4 = 5){
			return ( ( arg0 + arg1 ) * arg2 + arg3 / arg4 ) & arg0;
        }
    )");
}

TEST(Functions, Call_AnotherFunction) {
    validate_shader(R"(
        int helper_func( int k ){
            return k * k;
        }

        shader main(int arg0 = 0,
                    out int arg2 = 5){
			arg2 = helper_func( arg0 );
        }
    )");
}

static int factorial_reference(int k) {
    if (k == 0)
        return 1;
    return k * factorial_reference(k - 1);
};

TEST(Functions, Factorial) {
    auto shader_source = R"(
        int factorial( int k ){
            if( k == 0 )
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

static int fibonacci(int k) {
    if (k <= 1) return k;
    return fibonacci(k - 1) + fibonacci(k - 2);
}

TEST(Functions, Fibonacci) {
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