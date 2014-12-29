/**----------------------------------------------------------------------------
 * _test_boost_bind.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:25 14:45 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"

/**
* @brief	boost::bind() - part 1
*/
int f(int a, int b)
{
    return a+b;
}

int g(int a, int b, int c)
{
    return a+b+c;
}

bool boost_bind()
{
    std::cout << boost::bind(f, 1, 2)() << std::endl;           // f(1,2) 와 동일한 함수객체
    std::cout << boost::bind(g, 1,2,3)() << std::endl;          // g(1,2,3) 과 동일한 함수객체

    // boost::function 을 사용하면 bind() 된 함수객체를 저장할 수 있다!!
    //
	boost::function<int (int, int)> ff = boost::bind(f, 1, 2);
    std::cout	<< "ff(1,2) = "					<< ff(1,2) 
				<< "boost::bind(f, 1, 2)() = "	<< boost::bind(f, 1, 2)() 
				<< std::endl;

	// <짱 중요!!>
	// f 함수는 int(int, int) 타입이지만
	// boost::bind(f, 1, _1) 로 파라미터를 고정해서 f(val) 형태로 호출이 가능하고, 
	// 항상 (1+ val) 를 리턴하게 된다. 이런 특징을 이용해서 callback 함수에 부가적인 
	// 파라미터를 전달하거나, 파라미터 순서를 조작하는 등의 일을 할 수 있다.
	boost::function<int (int, int)> xf = boost::bind(f, 1, _1);
    std::cout	<< "xf(1, 2) = "					<< ff(1, 2) 
				<< "boost::bind(f, 1, _1)(2) = "	<< boost::bind(f, 1, _1)(2) 
				<< std::endl;


    int x = 3;
    int y = 4;
    int z = 5;

    assert( f(y, x) == bind(f, _2, _1)(x,y) );
    assert( g(x, 9, x) == bind(g, _1, 9, _1)(x) );
    assert( g(z,z,z) == bind(g, _3, _3, _3)(x,y,z) );
    assert( g(x,x,x) == bind(g, _1, _1, _1)(x,y,z) );

	return true;
}










/**
* @brief	boost::bind() - part 2 함수객체 바인드
*/
struct F
{
    int operator()(int a, int b) {return a-b;}
    bool operator()(long a, long b) {return a=b ? true : false;}
};

bool boost_bind2()
{
    F f;

    int x = 104;
    assert( 0 == boost::bind<int>(f, _1, _1)(x) );            // f(x, x)    

    long y = 104;
    assert( true == boost::bind<bool>(f, _1, _1)(y) );            // f(x, x)
    assert( true == boost::bind( boost::type<bool>(), f, _1, _1)(y) );

	return true;
}


/**
* @brief    boost::bind() - part 3 함수객체 바인드, 함수 객체 상태 처리
            - 내부 변수가 있는 경우 result_type 을 재정의 안하면 에러나네?
            - 상태를 보존하고 싶은 경우 boost::ref() 를 통해 레퍼런스로 넘겨야 함 
            - 디폴트로 함수객체를 bind() 가 복사해서 가지고 있음
*/
struct FF
{
    int s;
    typedef void result_type;
    void operator() (int x) { s += x; }
};

bool boost_bind3()
{
    FF f={0};
    int a[] = {1,2,3};

    std::for_each(a, a+3, boost::bind( boost::ref(f), _1) );
    assert( f.s == 6 );

	return true;
}



/**
* @brief	boost::bind() - part 4 클래스 메소드 바인딩
*/
typedef class A
{
private:    
    int m;
public:
    A(int val) : m(val) {}

    int add_value(int val) 
    { 
        m += val;
        std::cout << __FUNCTION__ << ", m=" << m << std::endl;
        return m;
    }
    
    void start(int v) 
    { 
        std::cout << __FUNCTION__ << ", m=" << m << ", v=" << v << std::endl;
    };
    
}*PA;


bool boost_bind4()
{
    std::vector<int> vint;
    for(int i = 0; i < 12; ++i)
    {
        vint.push_back(i);
    }

    /* 
    - class method 바인딩
    - 아래 코드와 동일
        std::vector<int>::iterator it = vint.begin();
        for(; vint.end(); ++it)
        {
            a.add_value(*it);
        }
    */
    A a(100);
    boost::function <int (int)> functor_add_value = boost::bind( &A::add_value, a, _1);
    std::for_each(vint.begin(), vint.end(), functor_add_value);
    //std::for_each(vint.begin(), vint.end(), boost::bind( &A::add_value, a, _1));
    
    
    /*    
    - class method 바인딩할때는 첫번째 파라미터가 class instance 이어야 함
    - std::vector<PA> 일때나 std::vector<A> 일때나 functor 가 잘 동작함

    - 아래 코드와 동일함
        std::vector<PA>::iterator it = vA.begin();
        for(; it != vA.end(); ++it)
        {
            it->start();
        }

    - std::for_each( ) 는 [ vA.begin(), vA.end() ) 를 boost::bind(&A::start, _1)(it) 로 호출이므로
      it::start() 와 동일함
    */
    std::vector<PA> vA;
    for (int i = 0; i < 10; ++i)
    {
        PA pa = new A(i);
        vA.push_back(pa);
    }
    
    int value = 0;
    std::for_each(vA.begin(), vA.end(), boost::bind(&A::start, _1, ++value) );    

	return true;
}

//=============================================================================
// boost bind 를 이용한 콜백함수 처리 예제
// 일반 함수 포인터를 선언하면 boost::bind() 로 처리가 안됨
// boost::function 으로 선언해주어야 함
// 

class A5
{
public:
    void print(const std::string &s) 
	{
        std::cout << s << std::endl;
    }
};


typedef boost::function<void()> callback;

class B5
{
public:
    void set_callback(callback cb) 
	{
        m_cb = cb;
    }

    void do_callback() 
	{
        m_cb();
    }

private:
    callback m_cb;
};

void regular_function() 
{
    std::cout << "regular!" << std::endl;
}

bool boost_bind5()
{
    A5 a;
    B5 b;
    std::string s("message");

    // you forget the "&" here before A::print!
    b.set_callback(boost::bind(&A5::print, &a, s));
    b.do_callback();

    // this will work for regular function pointers too, yay!
    b.set_callback(regular_function);
    b.do_callback();

	return true;
}