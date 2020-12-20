#pragma once

#include <iberbar/Utility/Platform.h>
#include <xmemory>

namespace iberbar
{
	// ��Ϸ�������
	// ���ڴ���һЩ�ص��ڲ�
	class __iberbarUtilityApi__ CBaseCommand abstract
	{
	public:
		virtual ~CBaseCommand() {}
		virtual void Execute() = 0;
	};

	struct UCommandQueueOptions
	{
		bool bSync;
	};

	class __iberbarUtilityApi__ CCommandQueue abstract
	{
	public:
		virtual ~CCommandQueue() {}
		virtual void AddCommand( CBaseCommand* pCommand ) = 0;
		virtual void Execute() = 0;
	};




	__iberbarUtilityApi__ CCommandQueue* CreateCommandQueue(
		const UCommandQueueOptions& Options,
		std::pmr::memory_resource* pMemoryRes );

	__iberbarUtilityApi__ void DestroyCommandQueue( CCommandQueue* pQueue );



}



