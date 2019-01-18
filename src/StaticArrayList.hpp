#ifndef STATICARRAYLIST_HPP_
#define STATICARRAYLIST_HPP_

#include <cstddef>

template<typename T, std::size_t sz> struct StaticArrayList
{
	T data[sz];
	T* iterator[sz + 1];											// +1 for end ()
	T** reverse_iterator[sz];
	std::size_t size;

	StaticArrayList () : data {}, iterator {}, reverse_iterator {}, size (0) {}

	/*StaticArrayList (T dataarray[sz]) : data (dataarray), iterator ({}), reverse_iterator ({}), size (sz)
	{
		for (size_t i = 0; i < size; ++i)
		{
			iterator[i] = &data[i];
			reverse_iterator = &iterator[i];
		}
	}*/

	StaticArrayList (const StaticArrayList& that) : size (0)
	{
		for (size_t i = 0; i < that.size; ++i)
		{
			iterator[i] = &data[i];
			reverse_iterator = &iterator[i];
			data[i] = *that.iterator[i];
			++size;
		}
	}

	StaticArrayList& operator= (const StaticArrayList& that)
	{
		size = 0;
		for (size_t i = 0; i < that.size; ++i)
		{
			iterator[i] = &data[i];
			reverse_iterator = &iterator[i];
			data[i] = *that.iterator[i];
			++size;
		}

		return *this;
	}

	T** begin () {return &iterator [0];}

	T** end () {return &iterator[size];}

	bool empty () const {return (size == 0);}

	T& operator[] (const size_t n) {return *iterator[n];}

	T& at (const size_t n) {return ((n >= 0) && (n < size) ? *iterator[n] : data[0]);}

	T& front () {return *iterator[0];}

	T& back () {return *iterator[size - 1];}

	void new_data_segment (T** iterator_ptr)
	{
		T* new_ptr = &data[0];
		if (!empty ())
		{
			new_ptr = iterator[sz - 1];			// Default: last segment
			for (size_t i = 0; i < sz; ++i)		// But look for the first free segment
			{
				if (!reverse_iterator[i])
				{
					new_ptr = &data[i];
					break;
				}
			}

		}

		*iterator_ptr = new_ptr;
		reverse_iterator[new_ptr - &data[0]] = iterator_ptr;
	}

	void push_back (const T& content)
	{
		T** end_ptr = (size < sz ? end () : end () - 1);
		new_data_segment (end_ptr);
		**end_ptr = content;
		if (size < sz) ++size;
	}

	void pop_back ()
	{
		if (!empty ())
		{
			T** last = end () - 1;
			reverse_iterator[*last - &data[0]] = nullptr;
			*last = nullptr;
			--size;
		}
	}

	T** erase (T** iterator_ptr)
	{
		T** end_iit = end ();

		if (!empty ())
		{
			if (iterator_ptr == end_iit - 1)
			{
				pop_back ();
				return end ();													// Return new(!) end
			}

			if ((iterator_ptr >= begin ()) && (iterator_ptr < end_iit))
			{
				reverse_iterator[*iterator_ptr - &data[0]] = nullptr;
				for (T** iit = iterator_ptr; iit < end_iit - 1; ++iit)
				{
					reverse_iterator[*(iit + 1) - &data[0]] = iit;
					*iit = *(iit + 1);
				}
				*(end_iit - 1) = nullptr;										// New end: nullptr
				--size;
				return iterator_ptr;
			}
		}

		return end_iit;
	}

	T** insert (T** iterator_ptr, const T& content)
	{
		T** end_iit = (size < sz ? end () : end () - 1);

		if (iterator_ptr >= end_iit)
		{
			push_back (content);
			return end () - 1;
		}

		if ((iterator_ptr >= begin ()) && (iterator_ptr < end_iit))
		{
			if (size == sz) reverse_iterator[*end_iit - &data[0]] = nullptr;

			for (T** iit = end_iit - 1; iit >= iterator_ptr; --iit)
			{
				reverse_iterator[*iit - &data[0]] = iit + 1;
				*(iit + 1) = *iit;
			}

			new_data_segment (iterator_ptr);
			**iterator_ptr = content;
			if (size < sz) ++size;
			return iterator_ptr;
		}

		else return end ();
	}

	void push_front (const T& content) {insert (begin (), content);}

	void pop_front () {erase (begin ());}

};

template<typename T, std::size_t sz> std::ostream &operator<<(std::ostream &output, StaticArrayList<T, sz>& list)
{
	for (T** it = list.begin (); it != list.end (); ++it) output << **it << " ";
    return output;
}



#endif /* STATICARRAYLIST_HPP_ */
