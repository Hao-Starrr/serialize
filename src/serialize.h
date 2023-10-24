#ifndef _SERIALIZE_HEADER_H_
#define _SERIALIZE_HEADER_H_
#include <sstream>   //std::stringstream
#include <vector>    //std::vector
#include <list>        //std::list
#include <set>       //std::set
#include <map>     //std::map
#include <utility>    // std::pair
#include <iterator>  //std::back_inserter
#include <string.h>  //memcpy

////////////////////////////////////////////////////
// define normal template function
////////////////////////////////////////////////////

/* 
这里在定义一个通用的模板函数。
用这个模板就可以用 serialize(xxx)去处理任何类，只要这个类里面定义了序列化的方法。
serialize(xxx)和序列化基本的数据类型的写法是相同的，
这样我要序列化一个东西的时候就不用管xxx到底是什么。

通用的情况是直接调用它的成员函数,
特殊的情况才使用int double 里面的写法。
*/

template<typename SerializableType>
std::string serialize ( SerializableType& a )
{
    return a.serialize ( );
}

template<typename SerializableType>
int deserialize ( std::string &str, SerializableType& a )
{
    return a.deserialize ( str );
}

/////////////////////////////////////////////////
//define special template function
//Serialize for C/C++ basic type
//examples: short,int,float,long long,double
/////////////////////////////////////////////////

/*

这里的写法非常秀操作，它全部是用宏来实现的。
在预处理阶段，这些宏就会被全部替换为给每一个类型对应的serialize函数。
如果你用重载，你就需要给每一种类型写一个函数。
但是你用宏，当再增加一种数据类型的时候，你就只需要在后面加一行。
这样他就不用给每一个类型写很多长得差不多的函数了。

但C++中并不推荐这种写法。更推荐的写法是用模板元编程。
他其实已经用了模板元编程，只不过他需要同时处理特殊的类型和自定义类型，他这里就只能用宏了。

并且它这里硬是不指定命名空间，每一次使用的时候都要用global的命名空间。

*/

#define DEF_BASIC_TYPE_SERIALIZE(Type) \
 template<> \
std::string inline serialize(Type& b) \
{ \
        std::string ret; \
        ret.append((const char*)&b,sizeof(Type)); \
        return ret; \
}

#define DEF_BASIC_TYPE_DESERIALIZE(Type)  \
 template<> \
int inline deserialize(std::string& str,Type& b)\
{ \
        memcpy(&b,str.data(),sizeof(Type)); \
        return sizeof(Type); \
}

#define DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE(Type) \
        DEF_BASIC_TYPE_SERIALIZE(Type) \
        DEF_BASIC_TYPE_DESERIALIZE(Type)

DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( char )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( unsigned char )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( short int )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( unsigned short int )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( unsigned int )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( int )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( long int )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( unsigned long int )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( float )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( long long int )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( unsigned long long int )
DEF_BASIC_TYPE_SERIALIZE_AND_DESERIALIZE ( double )

//////////////////////////////////////
//Serialize for type string
/////////////////////////////////////

// for c++ type std::string
template<>
std::string inline serialize ( std::string& s )
{
    int len = s.size ( );
    std::string ret;
    ret.append ( ::serialize ( len ) );
    ret.append ( s.data ( ), len );
    return ret;
}

template<>
int inline deserialize ( std::string &str, std::string& s )
{
    int len;
    ::deserialize ( str, len );
    s = str.substr ( sizeof (len ), len );
    return sizeof (int )+len;
}

////////////////////////////////////////////
//define input and output stream
//for serialize data struct
////////////////////////////////////////////

class OutStream
{
public:

    OutStream ( ) : os ( std::ios::binary )
    {
    }
    /*
    std::ostringstream os;是成员变量。
    在这里新建了一个成员变量，每次创建auto stream的时候就会自动新建这个成员变量，
    并指定了它的模式为二进制模式 。但这里二进制模式是没用的。
    */

    template<typename SerializableType>
    OutStream& operator<< ( SerializableType& a )
    {
        std::string x = ::serialize ( a );
        os.write ( x.data ( ), x.size ( ) );
        return *this;
    }

    template<typename BasicType>
    OutStream& operator<< ( std::vector<BasicType>& a )
    {
        int len = a.size ( );
        std::string x = ::serialize ( len );
        os.write ( x.data ( ), x.size ( ) );

        for ( int i = 0; i < len; ++i )
        {
            std::string item = ::serialize ( a[i] );
            os.write ( item.data ( ), item.size ( ) );
        }

        return *this;
    }

    template<typename BasicType>
    OutStream& operator<< ( std::list<BasicType>& a )
    {
        std::vector<BasicType> temp;
        std::copy ( a.begin ( ), a.end ( ), std::back_inserter ( temp ) );
        return this->operator<< ( temp );
    }
    /*

    我写我就会这样写:
    template<typename T>
    OutStream& operator<<(T& input)
    {
        std::string str = ::serialize(input);
        os.write(str.data(), str.size());
        return *this; // 我在class里面overload << 的时候 就不需要传入这个class了。
    }
    template<typename T>
    OutStream& operator<<(std::vector<T>& input)
    {
        int len = input.size()
        std::string str = ::serialize(len);
        os.write(str.data(), str.size());

        for(auto vec : input){ // 这里用到了C++新的特性auto和：
            std::string str = ::serialize(vec);
            os.write(str.data(), str.size());
        }
        return *this;
    }
    template<typename T>
    OutStream& operator<<(std::list<T>& inputList)
    {
        std::vector<T> myVector(inputList.begin(), inputList.end());
        // 这里是用一个list直接初始化一个vector。更简洁。
        return this->operator<< ( myVector );
    }

    */

    

    template<typename BasicType>
    OutStream& operator<< ( std::set<BasicType>& a )
    {
        std::vector<BasicType> temp;
        std::copy ( a.begin ( ), a.end ( ), std::back_inserter ( temp ) );
        return this->operator<< ( temp );
    }
    
    template<typename BasicTypeA, typename BasicTypeB>
    OutStream& operator<< ( std::map<BasicTypeA, BasicTypeB>& a )
    {
        std::vector<BasicTypeA> tempKey;
        std::vector<BasicTypeB> tempVal;
        
        typename std::map<BasicTypeA, BasicTypeB>::const_iterator it;
        for(it=a.begin();it!=a.end ();++it)
        {
            tempKey.push_back (it->first);
            tempVal.push_back (it->second);
        }
        
        this->operator<< ( tempKey );
        return this->operator<< ( tempVal );
    }

    std::string str ( )
    {
        return os.str ( );
    }

public:
    std::ostringstream os;
};

class InStream
{
public:

    InStream ( std::string &s ) : str ( s ), total ( s.size ( ) )
    {
    }

    template<typename SerializableType>
    InStream& operator>> ( SerializableType& a )
    {
        int ret = ::deserialize ( str, a );
        str = str.substr ( ret );
        return *this;
    }

    template<typename BasicType>
    InStream& operator>> ( std::vector<BasicType>& a )
    {
        int len = 0;
        int ret = ::deserialize ( str, len );
        str = str.substr ( ret );

        for ( int i = 0; i < len; ++i )
        {
            BasicType item;
            int size = ::deserialize ( str, item );
            str = str.substr ( size );
            a.push_back ( item );
        }

        return *this;
    }

    template<typename BasicType>
    InStream& operator>> ( std::list<BasicType>& a )
    {
        std::vector<BasicType> temp;
        InStream& ret = this->operator>> ( temp );
        if ( temp.size ( ) > 0 )
        {
            std::copy ( temp.begin ( ), temp.end ( ), std::back_inserter ( a ) );
        }

        return ret;
    }

    template<typename BasicType>
    InStream& operator>> ( std::set<BasicType>& a )
    {
        std::vector<BasicType> temp;
        InStream& ret = this->operator>> ( temp );
        if ( temp.size ( ) > 0 )
        {
            for ( size_t i = 0; i < temp.size ( ); ++i )
            {
                a.insert ( temp[i] );
            }
        }

        return ret;
    }

    template<typename BasicTypeA,typename BasicTypeB>
    InStream& operator>> ( std::map<BasicTypeA,BasicTypeB>& a )
    {
        std::vector<BasicTypeA> tempKey;
        std::vector<BasicTypeB> tempVal;
        
        this->operator>> ( tempKey );
        InStream& ret = this->operator>> ( tempVal );
        
        if ( tempKey.size ( ) > 0 && tempVal.size ()==tempKey.size () )
        {
            for ( size_t i = 0; i < tempKey.size ( ); ++i )
            {
                a.insert ( std::make_pair<BasicTypeA,BasicTypeB>(tempKey[i],tempVal[i]) );
            }
        }

        return ret;
    }

    int size ( )
    {
        return total - str.size ( );
    }

protected:
    std::string str;
    int total;
};

////////////////////////////////////////////
//Serialize for custom class object
//If your class object want to be serialized,
//Please derive for this base class
///////////////////////////////////////////

class Serializable
{
public:
    virtual std::string serialize ( ) = 0;
    virtual int deserialize ( std::string& ) = 0;
};

///////////////////////////////////////////
//!!!!!!!!!!!!!
//!!   NOTE  !!
//!!!!!!!!!!!!!
//Now,we can't serialize pointer type,likes
//char* var,char var[],int *var,int var[]
//etc,if you need to serialize array data,
//we suggest you use std::vector instead.
///////////////////////////////////////////

#endif
