#ifndef _AM_EXCEPTION_
#define _AM_EXCEPTION_

#ifdef __cplusplus

class AMException
{
public:
	AMException(): m_ret_code(-1), m_exception_desc(NULL){}
	AMException(int ret_code): m_ret_code(ret_code), m_exception_desc(NULL){}
	AMException(int ret_code, const TCHAR* desc): m_ret_code(ret_code), m_exception_desc(desc){}

	const TCHAR* Description(){return m_exception_desc;}
	int	RetCode(){return m_ret_code;} 

private:
	int				m_ret_code;			// please refer to the definition in RetCode.h
	const TCHAR*	m_exception_desc;	// the constant message of exception message
};

#endif

#endif
