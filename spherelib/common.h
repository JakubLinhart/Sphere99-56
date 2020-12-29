#pragma once

#define IMULDIV(a,b,c) (((a)*(b))/(c))	// windows MulDiv will round ! 

// use to indicate that a function uses printf-style arguments, allowing GCC
// to validate the format string and arguments:
// a = 1-based index of format string
// b = 1-based index of arguments
// (note: add 1 to index for non-static class methods because 'this' argument
// is inserted in position 1)
#ifdef __GNUC__
#define __printfargs(a,b) __attribute__ ((format(printf, a, b)))
#else
#define __printfargs(a,b)
#endif

#ifndef COUNTOF
#define COUNTOF(a)	(sizeof(a)/sizeof((a)[0]))
#endif

#define UID_INDEX DWORD
#define HASH_INDEX DWORD
#define HASH_COMPARE(a, b) (a>b)

#ifndef _1BITMASK
#define _1BITMASK(b)    (((size_t)1) << (b))
#endif

class CServTime
{
#undef GetCurrentTime
#define TICK_PER_SEC 10
    // A time stamp in the server/game world.
public:
    long m_lPrivateTime;
public:
    long GetTimeRaw() const
    {
        return m_lPrivateTime;
    }
    int GetTimeDiff(const CServTime& time) const
    {
        return(m_lPrivateTime - time.m_lPrivateTime);
    }
    void Init()
    {
        m_lPrivateTime = 0;
    }
    void InitTime(long lTimeBase)
    {
        m_lPrivateTime = lTimeBase;
    }
    bool IsTimeValid() const
    {
        return(m_lPrivateTime ? true : false);
    }
    CServTime operator+(int iTimeDiff) const
    {
        CServTime time;
        time.m_lPrivateTime = m_lPrivateTime + iTimeDiff;
        return(time);
    }
    CServTime operator-(int iTimeDiff) const
    {
        CServTime time;
        time.m_lPrivateTime = m_lPrivateTime - iTimeDiff;
        return(time);
    }
    int operator-(CServTime time) const
    {
        return(m_lPrivateTime - time.m_lPrivateTime);
    }
    bool operator==(CServTime time) const
    {
        return(m_lPrivateTime == time.m_lPrivateTime);
    }
    bool operator!=(CServTime time) const
    {
        return(m_lPrivateTime != time.m_lPrivateTime);
    }
    bool operator<(CServTime time) const
    {
        return(m_lPrivateTime < time.m_lPrivateTime);
    }
    bool operator>(CServTime time) const
    {
        return(m_lPrivateTime > time.m_lPrivateTime);
    }
    bool operator<=(CServTime time) const
    {
        return(m_lPrivateTime <= time.m_lPrivateTime);
    }
    bool operator>=(CServTime time) const
    {
        return(m_lPrivateTime >= time.m_lPrivateTime);
    }
    static CServTime GetCurrentTime();
};