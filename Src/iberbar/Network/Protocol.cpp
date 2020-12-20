
#include <iberbar/Network/Protocol.h>
#include <memory.h>




int iberbar::Net::SocketProtocol_FindHead( const char* pFd, size_t nFdLen, size_t nFdStart, const char* pHead, size_t nHeadLen, int& nMaybe )
{
    if ( nFdLen <= nFdStart )
    {
        nMaybe = -1;
        return -1;
    }

    size_t nFdLenNew = nFdLen - nFdStart;
    const char* pFdNew = pFd + nFdStart;
    while ( nFdLenNew >= nHeadLen )
    {
        if ( memcmp( pFdNew, pHead, nHeadLen ) == 0 )
        {
            return ((int)nFdLen - (int)nFdLenNew);
        }
        pFdNew++;
        nFdLenNew--;
    }

    if ( memcmp( pFdNew, pHead, (nHeadLen - nFdLenNew) ) == 0 )
    {
        nMaybe = ((int)nFdLen - (int)nFdLenNew);
    }
    else
    {
        nMaybe = -1;
    }

    return -1;
}






//
//
//int iberbar::Server::CSocketProtocol_HeadAndTail::Read( const char* pFd, size_t nFdLen )
//{
//    int nHeadPos, nMaybe;
//    while ( ( nHeadPos = SocketProtocol_FindHead( pFd, nFdLen, m_pHead, m_nHeadLen, nMaybe ) ) >= 0 )
//    {
//
//    }
//
//    if ( nMaybe < 0 )
//    {
//        return nFdLen;
//    }
//}

