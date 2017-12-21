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
#include <iostream>
#include <unordered_map>


bool test_std_unordered_map_object();
bool test_unorded_map_test_move();


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

	// 
	//	map.erase() 만 호출하면 item 의 소멸자는 호출되지 않으므로 
	//	erase 호출 전에 second 객체의 소멸자를 호출해주어야 한다. 
	// 
	std::map<DWORD, pmap_second_object>::iterator it = my_map.find(1);
	delete it->second;
	my_map.erase(it);

	for(std::map<DWORD, pmap_second_object>::iterator itl = my_map.begin();
		itl != my_map.end();
		++itl)
	{
		delete itl->second;
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

/// @brief
bool test_std_unordered_map()
{
    std::unordered_map<std::string, std::string> mymap;
    mymap = { { "Australia","Canberra" },{ "U.S.","Washington" },{ "France","Paris" } };

    // iterate items
    std::cout << "mymap contains:";
    for (auto it = mymap.begin(); it != mymap.end(); ++it)
        std::cout << " " << it->first << ":" << it->second;
    std::cout << std::endl;

    // iterate buckets
    std::cout << "---" << std::endl;
    std::cout << "mymap's buckets contain:\n";
    for (unsigned i = 0; i < mymap.bucket_count(); ++i) {
        std::cout << "bucket #" << i << " contains:";
        for (auto local_it = mymap.begin(i); local_it != mymap.end(i); ++local_it)
            std::cout << " " << local_it->first << ":" << local_it->second;
        std::cout << std::endl;
    }

    // add item
    std::cout << "---" << std::endl;
    mymap.insert(std::make_pair("aaa", "bbb"));
    std::cout << "mymap contains:";
    for (auto it = mymap.begin(); it != mymap.end(); ++it)
        std::cout << " " << it->first << ":" << it->second;
    std::cout << std::endl;

    // search item
    std::cout << "---" << std::endl;
    std::cout << "trying to find `aaa` : ";
    auto f = mymap.find("aaa");
    if (mymap.end() == f)
    {
        std::cout << "none" << std::endl;
    }
    else
    {
        std::cout << f->second << std::endl;
    }

    std::cout << "trying to find `bbb` : ";
    auto ff = mymap.find("bbb");
    if (mymap.end() == ff)
    {
        std::cout << "none" << std::endl;
    }
    else
    {
        std::cout << ff->second << std::endl;
    }

    return true;
}

/// @brief
typedef class MyP
{
public :
    MyP(uint32_t v)  : _v(v)
    {
        std::cout << __FUNCTION__ << std::endl;
    }
    virtual ~MyP() 
    {
        std::cout << __FUNCTION__ << "(" << _v << ")" << std::endl;
    }

    void dump()
    {
        std::cout << __FUNCTION__ << "(" << _v << ")" << std::endl;
    }

private:
    uint32_t _v;
} *PMyP;

typedef class MyC: public MyP
{
public:
    MyC(uint32_t v) : MyP(v)
    {
        std::cout << __FUNCTION__ << std::endl;
    }
    ~MyC() 
    {
        std::cout << __FUNCTION__ << std::endl;
    }
} *PMyC;

bool test_std_unordered_map_object()
{
    std::cout << "---" << std::endl;

    std::unordered_map<uint32_t, PMyP> mymap;

    // insert
    mymap.insert(std::make_pair < uint32_t, PMyP>(0, new MyC(0)));
    mymap.insert(std::make_pair < uint32_t, PMyP>(1, new MyC(1)));
    mymap.insert(std::make_pair < uint32_t, PMyP>(2, new MyC(2)));

    // iterate all objects
    for (auto& item : mymap)
    {
        item.second->dump();
    }

    // find and erase and free item.
    auto item = mymap.find(0);
    _ASSERTE(mymap.end() != item);
    delete item->second;
    _ASSERTE(1 == mymap.erase(item->first));        // erase using key

    item = mymap.find(1);
    _ASSERTE(mymap.end() != item);
    delete item->second;
    _ASSERTE(mymap.end() != mymap.erase(item));     // erase using iterator
    
    // trying to delete using invalid key 
    _ASSERTE(0 == mymap.erase(10000000));
    
    // clear the rest of all
    for (auto litem : mymap)
    {
        delete litem.second;
    }
    mymap.clear();






    // using [] operator
    std::cout << "---" << std::endl;

    mymap[0] = new MyC(0);
    mymap[1] = new MyC(1);
    for (auto itemx : mymap)
    {
        itemx.second->dump();
    }

    std::cout << "add MyC(2), using []" << std::endl;    
    mymap[2] = new MyC(2);

    for (auto itemx : mymap)
    {
        itemx.second->dump();
    }

    // access invalid item is equivalent adding new item...
    PMyP p = mymap[3];
    _ASSERTE(NULL == p);
    p = new MyC(3);
    mymap[3] = p;


    PMyP& pr = mymap[4];        // 참조자로 받을 수도 있다.
    _ASSERTE(NULL == pr);
    pr = new MyC(4);
    // mymap[3] = p;            <= 참조자로 받았기땜시롱, 값을 다시 설정할 필요없다.

    
    // clear the rest of all
    for (auto litem : mymap)
    {
        delete litem.second;
    }
    mymap.clear();
    
    _ASSERTE(0 == mymap.size());
    return true;
}





/// @brief	
typedef class SomeClass
{
public: 
    SomeClass(int v) : _v(v)
    {    
    }

    ~SomeClass()
    {
        printf("%s, v = %d\n", __FUNCTION__, _v);
    }

    int _v;

} *PSomeClass;

bool test_unorded_map_test_move()
{
	_CrtMemState memoryState = { 0 };
	_CrtMemCheckpoint(&memoryState);

	{
		typedef std::list<PSomeClass> SomeClassList, *PSomeClassList;
		std::unordered_map< int, PSomeClassList > um;



		PSomeClassList l1 = new SomeClassList();
		PSomeClass o11 = new SomeClass(11); l1->push_back(o11);
		PSomeClass o12 = new SomeClass(12); l1->push_back(o12);
		PSomeClass o13 = new SomeClass(13); l1->push_back(o13);
		PSomeClass o14 = new SomeClass(14); l1->push_back(o14);
		// make_pair< RValueRef, RValueRef > 가 와야 하는데, PSomeClassList 는 lvalue 이므로
		// 에러남. LValue 를 RValue 로 변환해주는 std::move() 를 이용해서 PSomeClassList 를 전달
		um.insert(std::make_pair<int, PSomeClassList>(1, std::move(l1)));



		PSomeClassList l2 = new SomeClassList;
		PSomeClass o21 = new SomeClass(21); l1->push_back(o21);
		PSomeClass o22 = new SomeClass(22); l1->push_back(o22);
		PSomeClass o23 = new SomeClass(23); l1->push_back(o23);
		PSomeClass o24 = new SomeClass(24); l1->push_back(o24);
		um.insert(std::make_pair<int, PSomeClassList>(2, std::move(l2)));


		um[2]->push_back(new SomeClass(31));
		um[2]->push_back(new SomeClass(32));
		um[2]->push_back(new SomeClass(33));
		um[2]->push_back(new SomeClass(34));


		// free all rsrc.
		for (auto item : um)
		{
			//PSomeClassList ln = item.second;
			auto ln = item.second;
			SomeClassList::iterator s = ln->begin();
			SomeClassList::iterator e = ln->end();
			for (; s != e; ++s)
			{
				delete *s;
			}
			ln->clear();
			delete ln;
		}
		um.clear();
	}

	_CrtMemDumpAllObjectsSince(&memoryState);
    return true;
}

bool test_map_insert_swap()
{
	std::unordered_map<int, std::string*> str_map;

	str_map.insert(std::make_pair<int, std::string*>(1, new std::string("1111")));
	str_map.insert(std::make_pair<int, std::string*>(2, new std::string("2222")));
	
	//
	//	새로운 엔트리를 추가하고, 기존 객체 제거하기 
	//
	std::string* pstr = new std::string("3333");
	std::swap(str_map[3], pstr);
	_ASSERTE(pstr == nullptr);	// 엔트리 추가이므로 nullptr 과 교환하게 됨


	//
	//	기존 엔트리를 다른 객체로 바꿔치기하고,
	//	기존 객체 제거하기 
	//

	_ASSERTE(pstr == nullptr);
	pstr = new std::string("1122");
	std::swap(str_map[1], pstr);
	
	//
	// pstr 에는 기존 str_map[1] 아이템 포인터가 있음
	//
	_ASSERTE(nullptr != pstr);
	_ASSERTE(0 == pstr->compare("1111"));
	log_info "delete %s", pstr->c_str() log_end;
	delete pstr;
	

	return true;
}