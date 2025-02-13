#include "pch.h"
#include "CppUnitTest.h"

#include "MemStreamBuf.h"

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <stdio.h>

#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s << std::endl;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;





namespace MemStreamTests
{
	TEST_CLASS(MemStreamTests)
	{
	private:
		MemStream::CMemStreamBuf<char> *memStreamPtr;

	public:
		
#pragma region Constructors
		TEST_METHOD(TestConstructorDestructor)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>();
			memStreamPtr->sync();
			delete memStreamPtr;
			assert(true);
		}
#pragma endregion Constructors

#pragma region sputc
		TEST_METHOD(Test_sputc_SingleCharacterInsert)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(1);
			memStreamPtr->sync();

			memStreamPtr->sputc('a');

			DBOUT(memStreamPtr->_buf->size());
			for (std::size_t i = 0; i < memStreamPtr->_buf->size(); ++i)
			{
				DBOUT(memStreamPtr->_buf->at(i));
			}

			auto buffer = memStreamPtr->_buf;
			assert(memStreamPtr->_buf->size() == 1);
			assert(buffer->at(0) == 'a');

			delete memStreamPtr;
			assert(true);
		}

#ifdef _CMEMSTREAMBUF_OVERFLOWBUF
		TEST_METHOD(Test_sputc_SingleCharBufOverFlow)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(1);
			memStreamPtr->sync();

			assert(memStreamPtr->sputc('a') != EOF);
			assert(memStreamPtr->sputc('b') == EOF);

			DBOUT(memStreamPtr->_buf->size());
			for (std::size_t i = 0; i < memStreamPtr->_buf->size(); ++i)
			{
				DBOUT(memStreamPtr->_buf->at(i));
			}
			DBOUT(memStreamPtr->_buf->size());

			assert(memStreamPtr->_buf->size() == 1);

			DBOUT(memStreamPtr->_overflowBuf->size());
			for (std::size_t i = 0; i < memStreamPtr->_overflowBuf->size(); ++i)
			{
				DBOUT(memStreamPtr->_overflowBuf->at(i));
			}
			DBOUT(memStreamPtr->_overflowBuf->size());

			assert(memStreamPtr->_overflowBuf->size() == 1);
			assert(memStreamPtr->_overflowBuf->at(0) == 'b');

			delete memStreamPtr;
			assert(true);
		}
#endif
#pragma endregion sputc

#pragma region xsputn
		TEST_METHOD(Test_xsputn_MultiCharPut)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(3);
			memStreamPtr->sync();

			std::vector<char> iVector;
			iVector.push_back('a');
			iVector.push_back('b');
			iVector.push_back('c');

			memStreamPtr->xsputn(iVector.data(), iVector.size());
			assert(memStreamPtr->_buf->size() == 3);
			for (std::size_t i = 0; i < memStreamPtr->_buf->size(); ++i)
			{
				DBOUT(memStreamPtr->_buf->at(i));
				assert(memStreamPtr->_buf->at(i) == iVector.at(i));
			}
			
			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_xsputn_MultiCharPutOverflow)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(3);
			memStreamPtr->sync();

			std::vector<char> iV;
			iV.push_back('a');
			iV.push_back('b');
			iV.push_back('c');
			iV.push_back('d');
			iV.push_back('e');
			iV.push_back('f');

			memStreamPtr->xsputn(iV.data(), iV.size());
			assert(memStreamPtr->_buf->size() == 3);
			for (std::size_t i = 0; i < memStreamPtr->_buf->size(); ++i)
			{
				DBOUT(memStreamPtr->_buf->at(i));
				assert(memStreamPtr->_buf->at(i) == iV.at(i));
			}

			assert(memStreamPtr->_overflowBuf->size() == 3);
			for (std::size_t i = 0; i < memStreamPtr->_overflowBuf->size(); ++i)
			{
				DBOUT("----------");
				DBOUT(i);
				DBOUT(i + 3);
				DBOUT(memStreamPtr->_overflowBuf->at(i));
				DBOUT(iV.at(i + 3));
				assert(memStreamPtr->_overflowBuf->at(i) == iV.at(i + 3));
			}

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_xsputn_MultiCharPut2Inserts)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(6);
			memStreamPtr->sync();

			std::vector<char> iV;
			iV.push_back('a');
			iV.push_back('b');
			iV.push_back('c');
			std::vector<char> iV2;
			iV2.push_back('d');
			iV2.push_back('e');
			iV2.push_back('f');

			memStreamPtr->xsputn(iV.data(), iV.size());
			memStreamPtr->xsputn(iV2.data(), iV2.size());

			assert(memStreamPtr->_buf->size() == 6);
			for (std::size_t i = 0; i < memStreamPtr->_buf->size(); ++i)
			{
				DBOUT(memStreamPtr->_buf->at(i));
				if (i < 3)
				{
					assert(memStreamPtr->_buf->at(i) == iV.at(i));
				}
				else 
				{
					assert(memStreamPtr->_buf->at(i) == iV2.at(i - 3));
				}
			}

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_xsputn_MultiCharPut2InsertsBiggerStream)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(100);
			memStreamPtr->sync();

			std::vector<char> iV;
			iV.push_back('a');
			iV.push_back('b');
			iV.push_back('c');
			std::vector<char> iV2;
			iV2.push_back('d');
			iV2.push_back('e');
			iV2.push_back('f');

			memStreamPtr->xsputn(iV.data(), iV.size());
			memStreamPtr->xsputn(iV2.data(), iV2.size());

			auto usize = memStreamPtr->GetUsedSize();


			assert(usize == 6);
			for (std::size_t i = 0; i < memStreamPtr->_buf->size(); ++i)
			{
				DBOUT(memStreamPtr->_buf->at(i));
				if (i < 3)
				{
					assert(memStreamPtr->_buf->at(i) == iV.at(i));
				}
				else
				{
					assert(memStreamPtr->_buf->at(i) == iV2.at(i - 3));
				}
			}

			delete memStreamPtr;
			assert(true);
		}
#pragma endregion xsputn
	};
}
