
NS_BASE_BEGIN

template<typename T>
T strTo(const std::string& str)
{
	T val;
    std::istringstream s(str);
    s >> val;

	return val;
}


template<typename T>
std::string toStr(const T& t)
{
    std::ostringstream s;
    s << t;
	
    return s.str();
}

NS_BASE_END


