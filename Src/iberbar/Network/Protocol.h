#pragma once

#include <iberbar/Utility/Ref.h>

namespace iberbar
{
	namespace Net
	{
		int SocketProtocol_FindHead( const char* pFd, size_t nFdLen, size_t nFdStart, const char* pHead, size_t nHeadLen, int& nMaybe );



		//class CProtocolBase
		//	: public CRef
		//{
		//public:
		//	virtual bool Read( const char* pFd, size_t nFdLen, size_t* pReadLen ) = 0;
		//};


		//bool CreateProtocol_HeadAndTail( CProtocolBase** ppOutProtocol, const char* pHead, const char* pTail );

		enum class UProtocolType : int
		{
			DEFAULT,
			HEAD_AND_TAIL
		};

		class CProtocolBase
			: public CRef
		{
		public:
			CProtocolBase()
				: m_nType( UProtocolType::DEFAULT )
				, m_pData( nullptr )
			{
			}
			virtual ~CProtocolBase()
			{
				SAFE_DELETE_ARRAY( m_pData );
			}
			inline UProtocolType GetType() { return m_nType; }
		protected:
			UProtocolType m_nType;
			char* m_pData;
		};


		class CProtocol_HeadAndTail
			: public CProtocolBase
		{
		public:
			CProtocol_HeadAndTail( const char* pHead, int nHeadLen, const char* pTail, int nTailLen )
				: m_pHead( nullptr )
				, m_nHeadLen( 0 )
				, m_pTail( nullptr )
				, m_nTailLen( 0 )
			{
				assert( pHead );
				assert( nHeadLen > 0 );
				assert( pTail );
				assert( nTailLen > 0 );

				m_nType = UProtocolType::HEAD_AND_TAIL;

				m_pData = new char[ nHeadLen + nTailLen ];

				memcpy_s( m_pData, nHeadLen, pHead, nHeadLen );
				m_pHead = m_pData;

				memcpy_s( m_pData + nHeadLen, nTailLen, pTail, nTailLen );
				m_pTail = m_pData + nHeadLen;

				m_nHeadLen = nHeadLen;
				m_nTailLen = nTailLen;
			}

		public:
			inline const char* Head() const { return m_pHead; }
			inline int HeadLen() const { return m_nHeadLen; }

			inline const char* Tail() const { return m_pTail; }
			inline int TailLen() const { return m_nTailLen; }

		protected:
			char* m_pHead;
			size_t m_nHeadLen;
			char* m_pTail;
			size_t m_nTailLen;
		};



		//class __iberbarServerApis__ ISocketProtolcol
		//{
		//public:
		//	virtual const char* Read( const char* pFd, size_t nFdLen, size_t& nBodyLen, size_t& nDrain ) = 0;
		//};



		//class __iberbarServerApis__ CSocketProtocol_HeadAndTail
		//	: public ISocketProtolcol
		//{
		//public:
		//	virtual int Read( const char* ptr, size_t nFdLen ) override;
		//private:
		//	char* m_pHead;
		//	size_t m_nHeadLen;
		//};
	}
}