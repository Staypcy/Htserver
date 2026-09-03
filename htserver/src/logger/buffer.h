#ifndef HTSERVER_INCLUDE_HTSERVER_BUFFER_H
#define HTSERVER_INCLUDE_HTSERVER_BUFFER_H

#include<algorithm>
#include<array>
#include<cstddef>
#include<cstring>
#include<string_view>
#include<streambuf>
#include<vector>

namespace htserver{

template<std::size_t INLINE_CAPACITY>
class InlineBuffer {
private:
    std::array<char, INLINE_CAPACITY> m_inline{};
    std::vector<char> m_overflow; //存放长日志
    std::size_t m_size=0;
};


template<std::size_t INLINE_CPACITY>
class SmallStreamBuffer final:public std::streambuf{
public:
    explicit SmallStreamBuffer(InlineBuffer<INLINE_CPACITY>& buffer)override
        :m_buffer(buffer)
    {}

protected:
    std::streamsize xputn(const char* data,std::streamsize size)override{
        if(size<=0){
            return 0;
        }

        m_buffer.append(data,static_cast<std::size_t>(size));
        return size;
    }

    int_type overflow(int_type character)override{
        if(traits_type::eq_int_type(character,traits_type::eof())){
            return traits_type::not_eof(character);
        }
        m_buffer.append(traits_type::to_char_type(character));
        return character;
    }

private:
    //这里采用引用，外部对象
    InlineBuffer<INLINE_CPACITY>& m_buffer;
};
}

#endif