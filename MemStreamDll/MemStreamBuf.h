// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the MEMSTREAM_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// MEMSTREAM_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MEMSTREAM_EXPORTS
#define MEMSTREAM_API __declspec(dllexport)
#define EXPIMP_TEMPLATE
#else
#define MEMSTREAM_API __declspec(dllimport)
#define EXPIMP_TEMPLATE extern
#endif

#include <cassert>
#include <cstring>
#include <vector>
#include <iostream>
#include <streambuf>

namespace MemStream
{

	//http://www.unknownroad.com/rtfm/VisualStudio/warningC4251.html
	//https://stackoverflow.com/questions/72402396/a-template-argument-list-is-not-allowed-in-a-declaration-of-a-primary-template
	//EXPIMP_TEMPLATE template class MEMSTREAM_API std::allocator<char>;
	//EXPIMP_TEMPLATE template class MEMSTREAM_API std::vector<char, std::allocator<char>>;

	// This class is exported from the dll
	template<class CharT = char, class Traits = std::char_traits<CharT>>
	class MEMSTREAM_API CMemStreamBuf : public std::basic_streambuf<CharT, Traits>
	{
	public:
		std::vector<CharT>* GetBuffer();

#ifdef NDEBUG
	protected:
#else
	public:
#endif
		std::vector<CharT>* _buf;
		typename std::vector<CharT>::size_type _capacity;

#ifdef _CMEMSTREAMBUF_OVERFLOWBUF
	public:
		std::vector<CharT>* _overflowBuf;
#endif

#ifdef NDEBUG
	private:
#else
	public:
#endif
		std::streambuf::off_type GetPtrIn(std::ios_base::seekdir dir);
		std::streambuf::pos_type GetPtrOut(std::ios_base::seekdir dir);

		
	public:
		CMemStreamBuf(typename std::vector<CharT>::size_type capacity = 0);
		~CMemStreamBuf();

		static std::streambuf::int_type to_int_type(std::streambuf::char_type c) noexcept
		{
			return static_cast<std::streambuf::int_type>(c);
		}

	//Utility
	public:
		// New method to retrieve used size
		std::streamsize GetUsedSize() const;
		//std::streamsize CMemStreamBuf<CharT, Traits>::GetUsedSize() const
		//{
		//	return _usedSize;
		//}

	protected:
		std::streamsize _usedSize = 0; // New member variable to track used size

	//streambuf
	public:

		//Positioning
		std::basic_streambuf<CharT, Traits>* setbuf(std::streambuf::char_type* s, std::streamsize n) override;
		std::streambuf::pos_type seekoff(std::streambuf::off_type off, std::ios_base::seekdir dir,
			std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
		std::streambuf::pos_type seekpos(std::streambuf::pos_type pos,
			std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
		int sync() override;

		//Get area
		std::streamsize xsgetn(std::streambuf::char_type* s, std::streamsize count) override;
		std::streambuf::int_type underflow() override;
		//Put area
		/*std::streambuf::int_type sputc(std::streambuf::char_type ch);*/
		std::streamsize xsputn(const std::streambuf::char_type* s, std::streamsize count) override;
		std::streambuf::int_type overflow(std::streambuf::int_type ch = Traits::eof()) override;
		//Putback
	};

	template <typename CharT>
	MEMSTREAM_API CMemStreamBuf<CharT>* CMemStreamBufFactory();
}

//extern MEMSTREAM_API int nMemStream;
//
//MEMSTREAM_API int fnMemStream(void);
