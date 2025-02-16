// MemStream.cpp : Defines the exported functions for the DLL.
//

//Code references:
//https://codereview.stackexchange.com/questions/138479/memory-streambuf-and-stream
//https://stackoverflow.com/questions/8644096/multiple-conditions-in-switch-case


#include "pch.h"
#include "framework.h"
#include "MemStreamBuf.h"
#include <algorithm>

//Macro expansion failed to compile so we have to use algorithm lib instead.
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace MemStream
{

	template <typename CharT>
	CMemStreamBuf<CharT>* CMemStreamBufFactory() {
		return new CMemStreamBuf<CharT>();
	}

	// Explicit instantiations for required types
	template MEMSTREAM_API CMemStreamBuf<char>* CMemStreamBufFactory<char>();
	template MEMSTREAM_API CMemStreamBuf<wchar_t>* CMemStreamBufFactory<wchar_t>();
	template MEMSTREAM_API CMemStreamBuf<uint8_t>* CMemStreamBufFactory<uint8_t>();

#pragma region Constructors
	template<class CharT, class Traits>
	CMemStreamBuf<CharT, Traits>::CMemStreamBuf(typename std::vector<CharT>::size_type capacity)
		: _buf(new std::vector<CharT>(capacity)), _capacity(capacity)
#ifdef _CMEMSTREAMBUF_OVERFLOWBUF
		, _overflowBuf(new std::vector<CharT>())
#endif
	{
		this->setg(_buf->data(), _buf->data(), _buf->data() + capacity);
		this->setp(_buf->data(), _buf->data() + capacity);
	}

	template<class CharT, class Traits>
	CMemStreamBuf<CharT, Traits>::~CMemStreamBuf()
	{
		if (_buf != nullptr) delete _buf;
	}

#pragma endregion Constructors

	template<class CharT, class Traits>
	std::vector<CharT>* CMemStreamBuf<CharT, Traits>::GetBuffer()
	{
		return _buf;
	}

	//template<class CharT, class Traits>
	//std::streamsize CMemStreamBuf<CharT, Traits>::GetUsedSize() const
	//{
	//	return _usedSize; 
	//}

#pragma region Positioning
	template<class CharT, class Traits>
	std::basic_streambuf<CharT, Traits>* CMemStreamBuf<CharT, Traits>::setbuf(std::streambuf::char_type* s, std::streamsize n)
	{
		auto const begin = s;
		auto const end = s + n;

		this->setg(begin, begin, end);
		this->setp(begin, end);

		return this;
	}

	template<class CharT, class Traits>
	std::streambuf::off_type CMemStreamBuf<CharT, Traits>::GetPtrIn(std::ios_base::seekdir dir)
	{
		switch (dir)
		{
		case std::ios_base::beg:
			return *(this->eback());
		case std::ios_base::end:
			return *(this->egptr());
		case std::ios_base::cur:
			return *(this->gptr());
		default:
			return std::streambuf::pos_type(std::streambuf::off_type(-1));
		}
	}

	template<class CharT, class Traits>
	std::streambuf::pos_type CMemStreamBuf<CharT, Traits>::GetPtrOut(std::ios_base::seekdir dir)
	{
		switch (dir)
		{
		case std::ios_base::beg:
			return *(this->pbase());
		case std::ios_base::end:
			return *(this->epptr());
		case std::ios_base::cur:
			return *(this->pptr());
		default:
			return std::streambuf::pos_type(std::streambuf::off_type(-1));
		}
	}

	template<class CharT, class Traits>
	std::streambuf::pos_type CMemStreamBuf<CharT, Traits>::seekoff(std::streambuf::off_type off, std::ios_base::seekdir dir,
		std::ios_base::openmode which)
	{
		std::streambuf::off_type pos = std::streambuf::pos_type(std::streambuf::off_type(-1));
		switch (which)
		{
		case std::ios_base::in & std::ios_base::out: //Case where position is valid for both the put and get window.
			pos = GetPtrIn(dir) + off; //Get the pointer for the given window with the given offset.
			//if (pos < this->egptr() - this->eback()) //If pos is inside the range of the get window.
			if (pos < this->egptr() - this->eback())
			{
				this->setg(this->eback(), this->eback() + pos, this->egptr()); //Move the position pointer inside the get window & reset window.
			}
			pos = GetPtrOut(dir) + off; //Get the pointer for the given window with the given offset.
			if (pos < this->epptr() - this->pbase()) //If pos is in the range of the put window.
			{
				this->setp(this->pbase(), this->epptr()); //Reset window area. Redundant?
				this->pbump(pos); //Move put pointer to position.
			}
			return pos;
			break;

		case std::ios_base::in:
			pos = GetPtrIn(dir) + off; //Get the pointer for the given window with the given offset.
			if (pos < this->egptr() - this->eback()) //If pos is inside the range of the get window.
			{
				this->setg(this->eback(), this->eback() + pos, this->egptr()); //Move the position pointer inside the get window & reset window.
				return pos;
			}
			break;

		case std::ios_base::out:
			pos = GetPtrOut(dir) + off; //Get the pointer for the given window with the given offset.
			if (pos < this->epptr() - this->pbase()) //If pos is in the range of the put window.
			{
				this->setp(this->pbase(), this->epptr()); //Reset window area. Redundant?
				this->pbump(pos); //Move put pointer to position.
				return pos;
			}
			break;
		default:
			break;
		}

		return pos;
	}

	template<class CharT, class Traits>
	std::streambuf::pos_type CMemStreamBuf<CharT, Traits>::seekpos(std::streambuf::pos_type pos,
		std::ios_base::openmode which)
	{
		switch (which)
		{
		case std::ios_base::in & std::ios_base::out: //Case where position is valid for both the put and get window.
			if ( pos < std::min(this->egptr(), this->epptr()) - std::max( this->eback(), this->pbase() ) ) //The positon needs to be valid for the area that both windows overlap.
			{
				this->setg(this->eback(), this->eback() + pos, this->egptr()); //Move the position pointer inside the get window & reset window.
				this->setp(this->pbase(), this->epptr()); //Reset window area. Redundant?
				this->pbump(pos); //Move put pointer to position.
				return pos;
			}
			break;

		case std::ios_base::in:
			if (pos < this->egptr() - this->eback()) //If pos is inside the range of the get window.
			{
				this->setg(this->eback(), this->eback() + pos, this->egptr()); //Move the position pointer inside the get window & reset window.
				return pos;
			}
			break;

		case std::ios_base::out:
			if (pos < this->epptr() - this->pbase()) //If pos is in the range of the put window.
			{
				this->setp(this->pbase(), this->epptr()); //Reset window area. Redundant?
				this->pbump(pos); //Move put pointer to position.
				return pos;
			}
			break;

		default:
			break;
		}

		return std::streambuf::pos_type(std::streambuf::off_type(-1));
	}

	template<class CharT, class Traits>
	int CMemStreamBuf<CharT, Traits>::sync()
	{
		return -1;
	}

#pragma endregion Positioning

#pragma region GetArea
	template<class CharT, class Traits>
	std::streamsize CMemStreamBuf<CharT, Traits>::xsgetn(std::streambuf::char_type* s, std::streamsize count)
	{
		auto const size(std::min(this->egptr() - this->gptr(), count));
		std::memcpy(s, this->gptr(), size);
		this->gbump(size);

		return this->egptr() == this->gptr() ? CMemStreamBuf<CharT, Traits>::traits_type::eof() : size;
	}


	template<class CharT, class Traits>
	std::streambuf::int_type CMemStreamBuf<CharT, Traits>::underflow()
	{
		// Check if the get area is empty
		if (this->gptr() == this->egptr())
		{
			// No data left in the get area

#ifdef _CMEMSTREAMBUF_OVERFLOWBUF
				//Grab from the overflow buffer if available.
			if (_overflowBuf)
			{
				if (_overflowBuf->size() > 0)
				{
					std::streambuf::int_type retVal = _overflowBuf->front();
					_overflowBuf->erase(_overflowBuf->begin());
					return retVal;
				}
			}
#endif
			return Traits::eof(); // Return end-of-file to indicate no characters are available
		}

		// Return the character at the current get pointer as an int_type
		return Traits::to_int_type(*this->gptr());
	}


#pragma endregion GetArea

#pragma region PutArea
	//template<class CharT, class Traits>
	//std::streambuf::int_type CMemStreamBuf<CharT, Traits>::sputc(std::streambuf::char_type ch)
	//{
	//	return Traits::to_int_type(ch);
	//}

	template<class CharT, class Traits>
	std::streamsize CMemStreamBuf<CharT, Traits>::xsputn(const std::streambuf::char_type* s, std::streamsize count)
	{
	// Ensure we do not write past both the put window and the physical buffer end
		auto const size = std::min({
			this->epptr() - this->pptr(),   // Space left in the put area
			_buf->data() + _buf->size() - this->pptr(),  // Space left in the entire buffer
			count  // Requested write size
			});

#ifdef _CMEMSTREAMBUF_OVERFLOWBUF
		//Check for overlow.
		if (size < count)
		{
			if (!_overflowBuf)
			{
				_overflowBuf = new std::vector<CharT>();
			}
			auto pOffset = s;
			auto a = pOffset + size;
			auto b = pOffset + count;
			auto c = count - size;
			_overflowBuf->insert(_overflowBuf->end(), pOffset + size, pOffset + count);
		}
#endif

		std::copy(s, s + size, _buf->begin() + (this->pptr() - _buf->data()));

		this->pbump(size);

		return this->epptr() == this->pptr() ? CMemStreamBuf<CharT, Traits>::traits_type::eof() : size;
	}

	template<class CharT, class Traits>
	std::streambuf::int_type CMemStreamBuf<CharT, Traits>::overflow(std::streambuf::int_type ch)
	{
		// Check if we're trying to put an EOF (End-Of-File) character.
		if (Traits::eq_int_type(ch, Traits::eof())) {
			return Traits::not_eof(ch); // Indicate that no character has been consumed but not as EOF.
		}

		// Ensure the buffer exists.
		if (!_buf) {
			_buf = new std::vector<CharT>();
			_buf->reserve(this->_capacity); // Reserve memory based on static capacity.
		}

		//auto a = this->pbase();
		//auto b = this->pptr();
		//auto c = this->epptr();


		// Check if the buffer has reached its capacity.
		if (_buf->size() >= this->_capacity) 
		{
			// Buffer is full; we cannot accommodate the new character.

#ifdef _CMEMSTREAMBUF_OVERFLOWBUF
			//Add to overflow buffer if enabled.
			if (!_overflowBuf)
			{
				_overflowBuf = new std::vector<CharT>();
			}
			_overflowBuf->push_back(ch);
#endif
			
			return Traits::eof(); // Indicate that no more data can be written.
		}

		// Write the character directly to the buffer at the current put pointer
		*this->pptr() = Traits::to_char_type(ch);
		this->pbump(1);

		//Correctly update `_usedSize`
		_usedSize++;

		// Successfully consumed the character.
		return Traits::to_int_type(ch);
	}

#pragma endregion PutArea

}
