

class IPInfo
{
	public:
		IPInfo();
		~IPInfo();
	
		std::string IPAddress;
		std::string MACCurrent;
		struct timespec FirstSeen;
		struct timespec LastSeen;
};

