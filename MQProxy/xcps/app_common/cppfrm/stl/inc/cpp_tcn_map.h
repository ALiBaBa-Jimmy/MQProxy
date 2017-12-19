/************************************************************************************
文件名  :cpp_tcn_map.h

文件描述:

作者    :zzt

创建日期:2006/4/27

修改记录:
************************************************************************************/
#ifndef __CPP_TCN_MAP_H_
#define __CPP_TCN_MAP_H_

#include "cpp_prealloc.h"
#include "cpp_tcn_rb_tree.h"   //红黑树的定义

namespace tcn
{
    template <class TPAIR> struct CExtractFirst
    {
        const typename TPAIR::first_type& operator()(const TPAIR& x) const
        {
            return x.first;
        }
    };

    template <class Key, class T, class Compare = std::less<Key>, class Alloc = CAllocator>
    class map
    {
    public:
        typedef Key key_type;
        typedef T data_type;
        typedef T mapped_type;
        typedef std::pair<const Key, T> value_type;
        typedef Compare key_compare;

        class value_compare: public std::binary_function<value_type, value_type, bool>
        {
            friend class map<Key, T, Compare, Alloc>;
        protected :
            Compare comp;
            value_compare(Compare c) : comp(c)
            {
            }
        public:
            bool operator()(const value_type& x, const value_type& y) const
            {
                return comp(x.first, y.first);
            }
        };

    private:
        typedef rb_tree<key_type, value_type, CExtractFirst<value_type>, key_compare, Alloc> rep_type;
        rep_type t;
    public:
        typedef typename rep_type::pointer pointer;
        typedef typename rep_type::const_pointer const_pointer;
        typedef typename rep_type::reference reference;
        typedef typename rep_type::const_reference const_reference;
        typedef typename rep_type::iterator iterator;
        typedef typename rep_type::const_iterator const_iterator;

        typedef typename rep_type::size_type size_type;
        typedef typename rep_type::difference_type difference_type;
        //构造函数,使用内存预分配的配置器
        map(size_t tMaxCnt)
            :t(tMaxCnt,Compare())
        {
        }
        //构造函数，不使用内存预分配的配置器
        map()
            :t(Compare())
        {
        }

        //构造函数，不使用内存预分配的配置器
        explicit map(const Compare& comp,size_t tMaxCnt)
            :t(tMaxCnt,comp)
        {
        }

        //构造函数，不使用内存预分配的配置器
        explicit map(const Compare& comp)
            :t(comp)
        {
        }

        //构造函数，使用内存预分配的配置器
        template <class InputIterator>
        map(InputIterator first, InputIterator last,size_t tMaxCnt)
            :t(tMaxCnt,Compare())
        {
            t.insert_unique(first, last);
        }
        //构造函数，不使用内存预分配的配置器
        template <class InputIterator>
        map(InputIterator first, InputIterator last)
            :t(Compare())
        {
            t.insert_unique(first, last);
        }
        //拷贝构造
        map(const map<Key, T, Compare, Alloc>& x)
            :t(x.t)
        {

        }

        // 用于内存预分配,用户可以在构造函数中不提供预分配大小,
        // 接着使用reserve重新保留大小,使用方法和vector类似
        // add by liusj
        XVOID reserve(size_type n)
        {
            t.reserve(n);
        }

        map<Key, T, Compare, Alloc>& operator=(const map<Key, T, Compare, Alloc>& x)
        {
            t = x.t;//这里不用判断this != &x,因为t的定义中已经判断了
            return *this;
        }

        key_compare key_comp() const
        {
            return t.key_comp();
        }
        value_compare value_comp() const
        {
            return value_compare(t.key_comp());
        }
        iterator begin()
        {
            return t.begin();
        }
        const_iterator begin() const
        {
            return t.begin();
        }
        iterator end()
        {
            return t.end();
        }
        const_iterator end() const
        {
            return t.end();
        }


        bool empty() const
        {
            return t.empty();
        }
        size_type size() const
        {
            return t.size();
        }
        size_type max_size() const
        {
            return t.max_size();
        }
        T& operator[](const key_type& k)
        {
            return (*((insert(value_type(k, T()))).first)).second;
        }
        XVOID swap(map<Key, T, Compare, Alloc>& x)
        {
            t.swap(x.t);
        }


        std::pair<iterator,bool> insert(const value_type& x)
        {
            return t.insert_unique(x);
        }
        iterator insert(iterator position, const value_type& x)
        {
            return t.insert_unique(position, x);
        }
        template <class InputIterator>
        XVOID insert(InputIterator first, InputIterator last)
        {
            t.insert_unique(first, last);
        }


        XVOID erase(iterator position)
        {
            t.erase(position);
        }
        size_type erase(const key_type& x)
        {
            return t.erase(x);
        }
        XVOID erase(iterator first, iterator last)
        {
            t.erase(first, last);
        }
        XVOID clear()
        {
            t.clear();
        }


        iterator find(const key_type& x)
        {
            return t.find(x);
        }
        const_iterator find(const key_type& x) const
        {
            return t.find(x);
        }
        size_type count(const key_type& x) const
        {
            return t.count(x);
        }
        iterator lower_bound(const key_type& x)
        {
            return t.lower_bound(x);
        }

        const_iterator lower_bound(const key_type& x) const
        {
            return t.lower_bound(x);
        }
        iterator upper_bound(const key_type& x)
        {
            return t.upper_bound(x);
        }
        const_iterator upper_bound(const key_type& x) const
        {
            return t.upper_bound(x);
        }

        std::pair<iterator,iterator> equal_range(const key_type& x)
        {
            return t.equal_range(x);
        }
        std::pair<const_iterator,const_iterator> equal_range(const key_type& x) const
        {
            return t.equal_range(x);
        }

    #if defined (XOS_LINUX) ||defined (XOS_WIN32)
        friend bool operator== (const map&, const map&);
        friend bool operator< (const map&, const map&);
    #else
        friend bool operator== <Key, T, Compare, Alloc>(const map<Key, T, Compare, Alloc>&, const map<Key, T, Compare, Alloc>&);
        friend bool operator< <Key, T, Compare, Alloc> (const map<Key, T, Compare, Alloc>&, const map<Key, T, Compare, Alloc>&);
    #endif
    };

    template <class Key, class T, class Compare, class Alloc>
    inline bool operator==(const map<Key, T, Compare, Alloc>& x, const map<Key, T, Compare, Alloc>& y)
    {
        return x.t == y.t;
    }

    template <class Key, class T, class Compare, class Alloc>
    inline bool operator<(const map<Key, T, Compare, Alloc>& x,
                          const map<Key, T, Compare, Alloc>& y)
    {
        return x.t < y.t;
    }

    template <class Key, class T, class Compare, class Alloc>
    inline XVOID swap(map<Key, T, Compare, Alloc>& x,
                     map<Key, T, Compare, Alloc>& y)
    {
        x.swap(y);
    }

}//namespace std end

#endif

