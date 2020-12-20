#pragma once

#include <mutex>
#include <list>


namespace iberbar
{
	// ��Ϣ����
	// �̰߳�ȫ
	template < typename TMsg, typename TAllocator = std::allocator<TMsg> >
	class TMsgQueue
	{
	public:
		TMsgQueue();
		TMsgQueue( const TAllocator& Al );
		TMsgQueue( const TMsgQueue& Queue ) = delete;
		~TMsgQueue();

	public:
		// �����Ϣ������Ĭ�Ͽ���
		void AddMsg( const TMsg& Msg );
		// �����ɼ�
		bool PopMsg( TMsg& Msg );
		// ����
		void Lock();
		//  ����
		bool TryLock();
		// ����
		void Unlock();
		// �ж��Ƿ��
		bool IsEmpty() const;

	private:
		std::list<TMsg, TAllocator> m_MsgList;
		std::mutex m_Mutex;
	};
}


template < typename TMsg, typename TAllocator >
iberbar::TMsgQueue<TMsg, TAllocator>::TMsgQueue()
	: m_MsgList()
	, m_Mutex()
{
}


template < typename TMsg, typename TAllocator >
iberbar::TMsgQueue<TMsg, TAllocator>::TMsgQueue( const TAllocator& Al )
	: m_MsgList( Al )
	, m_Mutex()
{
}


template < typename TMsg, typename TAllocator >
iberbar::TMsgQueue<TMsg, TAllocator>::~TMsgQueue()
{
}


template < typename TMsg, typename TAllocator >
inline void iberbar::TMsgQueue<TMsg, TAllocator>::AddMsg( const TMsg& Msg )
{
	m_MsgList.push_back( Msg );
}


template < typename TMsg, typename TAllocator >
inline bool iberbar::TMsgQueue<TMsg, TAllocator>::PopMsg( TMsg& Msg )
{
	if ( m_MsgList.empty() == true )
		return false;
	Msg = m_MsgList.front();
	m_MsgList.pop_front();
	return true;
}


template < typename TMsg, typename TAllocator >
inline void iberbar::TMsgQueue<TMsg, TAllocator>::Lock()
{
	m_Mutex.lock();
}


template < typename TMsg, typename TAllocator >
inline bool iberbar::TMsgQueue<TMsg, TAllocator>::TryLock()
{
	return m_Mutex.try_lock();
}


template < typename TMsg, typename TAllocator >
inline void iberbar::TMsgQueue<TMsg, TAllocator>::Unlock()
{
	m_Mutex.unlock();
}


template < typename TMsg, typename TAllocator >
inline bool iberbar::TMsgQueue<TMsg, TAllocator>::IsEmpty() const
{
	return m_MsgList.empty();
}
