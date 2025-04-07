#include "pch.h"
#include "CppUnitTest.h"

#include "MemStreamBuf.h"

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <array>

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

		std::vector<char> _alphabet = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 
			'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

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

#pragma endregion sputc

#pragma region xsputn
		TEST_METHOD(Test_xsputn_MultiCharPut)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(3);
			memStreamPtr->sync();

			std::vector<char> iVector = { 'a', 'b', 'c' };

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

			std::vector<char> iV = { 'a', 'b', 'c', 'd', 'e', 'f' };

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

			std::vector<char> iV = { 'a', 'b', 'c' };
			std::vector<char> iV2 = { 'd', 'e', 'f' };

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

			std::vector<char> iV = {'a', 'b', 'c' };
			std::vector<char> iV2 = { 'd', 'e', 'f' };

			memStreamPtr->xsputn(iV.data(), iV.size());
			memStreamPtr->xsputn(iV2.data(), iV2.size());

			auto usize = memStreamPtr->GetUsedSize();

			assert(usize == 6);
			for (std::size_t i = 0; i < usize; ++i)
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

#pragma region sgetc
		TEST_METHOD(Test_sgetc_SingleCharGet)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(1);
			memStreamPtr->sync();
			std::vector<char> iV = {'a'};

			memStreamPtr->xsputn(iV.data(), iV.size());

			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == 1);
			
			auto res = memStreamPtr->sgetc();
			assert('a' == res);

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_sgetc_SingleCharGet_2CharTest)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(2);
			memStreamPtr->sync();
			std::vector<char> iV = { 'a', 'b' };
			memStreamPtr->xsputn(iV.data(), iV.size());

			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == 2);
			assert(usize == iV.size());

			auto res = memStreamPtr->sgetc();
			auto val = iV.at(0);
			assert(val == res);

			res = memStreamPtr->sgetc();
			val = iV.at(1);
			assert(val != res);

			delete memStreamPtr;
			assert(true);
		}
#pragma endregion sgetc

#pragma region sbumpc
		TEST_METHOD(Test_sbumpc_SingleCharGet)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(1);
			memStreamPtr->sync();
			std::vector<char> iV = { 'a' };

			memStreamPtr->xsputn(iV.data(), iV.size());

			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == 1);

			auto res = memStreamPtr->sbumpc();
			assert('a' == res);

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_sbumpc_2CharGet)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(2);
			memStreamPtr->sync();
			std::vector<char> iV = { 'a', 'b' };
			memStreamPtr->xsputn(iV.data(), iV.size());

			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == 2);
			assert(usize == iV.size());

			for (std::size_t i = 0; i < usize; ++i)
			{
				auto res = memStreamPtr->sbumpc();
				auto val = iV.at(i);
				assert(val == res);
			}

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_sbumpc_3CharGet)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(3);
			memStreamPtr->sync();
			std::vector<char> iV = { 'a', 'b', 'c' };
			memStreamPtr->xsputn(iV.data(), iV.size());

			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == 3);
			assert(usize == iV.size());

			for (std::size_t i = 0; i < usize; ++i)
			{
				auto res = memStreamPtr->sbumpc();
				auto val = iV.at(i);
				assert(val == res);
			}

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_sbumpc_AlphabetChars)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(_alphabet.size());
			memStreamPtr->sync();

			memStreamPtr->xsputn(_alphabet.data(), _alphabet.size());
			
			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == _alphabet.size());

			for (std::size_t i = 0; i < usize; ++i)
			{
				auto res = memStreamPtr->sbumpc();
				auto val = _alphabet.at(i);
				assert(val == res);
			}

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_sbumpc_AlphabetCharsLargeArray)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(100);
			memStreamPtr->sync();

			memStreamPtr->xsputn(_alphabet.data(), _alphabet.size());

			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == _alphabet.size());

			for (std::size_t i = 0; i < usize; ++i)
			{
				auto res = memStreamPtr->sbumpc();
				auto val = _alphabet.at(i);
				assert(val == res);
			}

			delete memStreamPtr;
			assert(true);
		}
#pragma endregion sbumpc

#pragma region sgetn
		TEST_METHOD(Test_sgetn_1CharGet)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(1);
			memStreamPtr->sync();

			std::vector<char> v1 = { 'a' };

			memStreamPtr->xsputn(v1.data(), v1.size());
			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == 1);

			std::array<char, 1> buffer;
			auto readCount = memStreamPtr->sgetn(buffer.data(), 1);
			assert(readCount == 1);
			assert(buffer.at(0) == 'a');

			readCount = memStreamPtr->sgetn(buffer.data(), 1);
			assert(readCount == EOF);

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_sgetn_1CharGetLargeStream)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(100);
			memStreamPtr->sync();

			std::vector<char> v1 = { 'a' };

			memStreamPtr->xsputn(v1.data(), v1.size());
			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == 1);

			std::array<char, 1> buffer;
			auto readCount = memStreamPtr->sgetn(buffer.data(), 1);
			assert(readCount == 1);
			assert(buffer.at(0) == 'a');

			readCount = memStreamPtr->sgetn(buffer.data(), 1);
			assert(readCount == 0);

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_sgetn_2CharStream1Get)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(2);
			memStreamPtr->sync();

			std::vector<char> v1 = { 'a', 'b' };

			memStreamPtr->xsputn(v1.data(), v1.size());
			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == 2);

			for (std::size_t i = 0; i < 3; ++i)
			{
				std::array<char, 1> buffer;
				auto readCount = memStreamPtr->sgetn(buffer.data(), 1);
				if (i < 2)
				{
					assert(readCount == 1);
					assert(buffer.at(0) == v1.at(i));
				}
				else
				{
					assert(readCount == EOF);
				}
			}

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_sgetn_3CharStream2Get)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(3);
			memStreamPtr->sync();

			std::vector<char> v1 = { 'a', 'b' };

			memStreamPtr->xsputn(v1.data(), v1.size());
			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == 2);

			for (std::size_t i = 0; i < 2; ++i)
			{
				std::array<char, 2> buffer;
				auto readCount = memStreamPtr->sgetn(buffer.data(), 2);
				if (i < 1)
				{
					assert(readCount == 2);
					for (std::size_t j = 0; j < buffer.size(); j++)
					{
						assert(buffer.at(j) == v1.at(j));
					}
				}
				else
				{
					assert(readCount == 0);
				}
			}

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_sgetn_100CharStream4Write2Get)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(100);
			memStreamPtr->sync();

			std::vector<char> v1 = { 'a', 'b' , 'c', 'd' };

			memStreamPtr->xsputn(v1.data(), v1.size());
			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == 4);

			for (std::size_t i = 0; i < 3; ++i)
			{
				std::array<char, 2> buffer;
				auto readCount = memStreamPtr->sgetn(buffer.data(), 2);
				if (i < 2)
				{
					assert(readCount == 2);
					for (std::size_t j = 0; j < buffer.size(); j++)
					{
						assert(buffer.at(j) == v1.at(j + 2 * i));
					}
				}
				else
				{
					assert(readCount == 0);
				}
			}

			delete memStreamPtr;
			assert(true);
		}

		TEST_METHOD(Test_sgetn_AlphabetCharStreamAlphabetWrite3Get)
		{
			memStreamPtr = new MemStream::CMemStreamBuf<char>(100);
			memStreamPtr->sync();

			auto dataSize = _alphabet.size();
			memStreamPtr->xsputn(_alphabet.data(), dataSize);
			auto usize = memStreamPtr->GetUsedSize();
			assert(usize == dataSize);

			auto iterCount = (int)dataSize / 3 + 1;
			for (std::size_t i = 0; i < iterCount; ++i)
			{
				std::array<char, 3> buffer;
				auto readCount = memStreamPtr->sgetn(buffer.data(), 3);
				if (i < iterCount - 1)
				{
					assert(readCount == 3);
					for (std::size_t j = 0; j < readCount; j++)
					{
						auto var1 = buffer.at(j);
						auto var2 = _alphabet.at(j + 3 * i);
						assert(buffer.at(j) == _alphabet.at(j + 3 * i));
					}
				}
				else if (i == iterCount - 1)
				{
					assert(readCount == 2);
					for (std::size_t j = 0; j < readCount; j++)
					{
						assert(buffer.at(j) == _alphabet.at(j + 3 * i));
					}
				}
				else
				{
					assert(readCount == 0);
				}
			}

			delete memStreamPtr;
			assert(true);
		}
#pragma endregion sgetn
	};
}
