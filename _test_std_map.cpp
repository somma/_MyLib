/**----------------------------------------------------------------------------
 * _test_std_map.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:1:25 14:54 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"

/**
* @brief	for test_std_map
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
typedef class map_second_object
{
public:
	map_second_object(DWORD idx) : _idx(idx)
	{
		std::cout << boost::format("%s") % __FUNCTION__ << std::endl;
	}
	~map_second_object()
	{
		std::cout << boost::format("%s") % __FUNCTION__ << std::endl;
	}

	DWORD _idx;

} *pmap_second_object;

bool test_std_map()
{
	std::map<DWORD, pmap_second_object> my_map;

	pmap_second_object obj = NULL;
	for(int i = 0; i < 4; ++i)
	{
		obj = new map_second_object(i);
		my_map.insert( std::make_pair(i, obj) );
	}

	//getchar();

	// 소멸자가 호출되지 않음, memory leak
	my_map.erase(0);		

	// 객체의 소멸자를 강제로 호출 후 erase 호출해야 함
	std::map<DWORD, pmap_second_object>::iterator it = my_map.find(1);
	delete it->second;
	my_map.erase(it);

	for(std::map<DWORD, pmap_second_object>::iterator it = my_map.begin();
		it != my_map.end();
		++it)
	{
		delete it->second;
	}
	my_map.clear();

	return true;
}

/**
* @brief	for test_map_plus_algorithm  (http://yesarang.tistory.com/348)
* @param	
* @see		
* @remarks	
* @code		
* @endcode	
* @return	
*/
class Elem {
public:
    Elem() : m_name(), m_val() {}    
    Elem(const std::string& name, int val) : m_name(name), m_val(val) {}    
    
	const std::string& GetName()const {return m_name;}
    int GetValue()				const {return m_val;}    
    void SetValue(int val)			  {m_val = val;}    
    void Print()				const {std::cout << GetName() << "'s value ==> " << GetValue() << std::endl;}
private:
    std::string m_name;
    int m_val;
};

bool test_map_plus_algorithm_1()
{
	std::map<std::string, Elem> mm;
	mm["1"] = Elem("1", 1);
	mm["2"] = Elem("2", 2);
	mm["3"] = Elem("3", 3);
	mm["4"] = Elem("4", 4);
	mm["5"] = Elem("5", 5);

	// map 에 담긴 모든 elem.m_val 을 얻어내고 싶다!
	std::vector<int> vi;
	for(std::map<std::string, Elem>::iterator it = mm.begin(); it != mm.end(); ++it)
	{
		vi.push_back(it->second.GetValue());
	}

	for(std::vector<int>::iterator iti = vi.begin(); iti != vi.end(); ++iti)
	{
		std::cout << "value=" << *iti << std::endl;
	}

	return true;
}




/**
 * @brief	test_map_plus_algorithm_2
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
struct ElemGetValueAdapter
{
	int operator() (const std::pair<std::string, Elem>& pair)
	{
		return pair.second.GetValue();
	}
};

bool test_map_plus_algorithm_2()
{
	std::map<std::string, Elem> mm;
	mm["1"] = Elem("1", 1);
	mm["2"] = Elem("2", 2);
	mm["3"] = Elem("3", 3);
	mm["4"] = Elem("4", 4);
	mm["5"] = Elem("5", 5);

	// map 에 담긴 모든 elem.m_val 을 얻어내고 싶다!
	// transform() 이용
	std::vector<int> vi(mm.size());
	std::transform(mm.begin(), mm.end(), vi.begin(), ElemGetValueAdapter());
	
	for(std::vector<int>::iterator iti = vi.begin(); iti != vi.end(); ++iti)
	{
		std::cout << "value=" << *iti << std::endl;
	}

	return true;
}



/**
 * @brief	test_map_plus_algorithm_3
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
struct ElemPrintAdaptor
{
	int operator() (const std::pair<std::string, Elem>& pair)
	{
		pair.second.Print();
		return 0;
	}
};

bool test_map_plus_algorithm_3()
{
	std::map<std::string, Elem> mm;
	mm["1"] = Elem("1", 1);
	mm["2"] = Elem("2", 2);
	mm["3"] = Elem("3", 3);
	mm["4"] = Elem("4", 4);
	mm["5"] = Elem("5", 5);

	// mm 에 속한 모든 Elem.Print 를 호출하고 싶다.
	// transform() 이용
	std::vector<int> vi(mm.size());
	std::transform(mm.begin(), mm.end(), vi.begin(), ElemPrintAdaptor());

	return true;
}



/**
 * @brief	test_map_plus_algorithm_4
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void print_value(int v)
{
	std::cout << "value = " << v << std::endl;
}

bool test_map_plus_algorithm_4()
{
	std::map<std::string, Elem> mm;
	mm["1"] = Elem("1", 1);
	mm["2"] = Elem("2", 2);
	mm["3"] = Elem("3", 3);
	mm["4"] = Elem("4", 4);
	mm["5"] = Elem("5", 5);

	// mm 에 속한 모든 Elem.Print 를 호출하고 싶다.
	// transform() 이용
	std::vector<int> vi(mm.size());
	std::transform(
				mm.begin(), 
				mm.end(), 
				vi.begin(), 
				boost::bind(
						&Elem::GetValue, 
						boost::bind(
								&std::map<std::string, Elem>::value_type::second, 
								_1)));

	std::for_each(vi.begin(), vi.end(), boost::bind(&print_value, _1));
	
	std::for_each(
				mm.begin(), 
				mm.end(), 
				boost::bind(
						&Elem::Print, 
						boost::bind(
								&std::map<std::string, Elem>::value_type::second, 
								_1)));

	return true;
}
