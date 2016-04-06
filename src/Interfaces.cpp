
#include <main.h>

std::list<std::string> Interfaces::List()
{
	std::list<std::string> lst;
	struct ifaddrs *iflst= NULL;

	int ret = getifaddrs(&iflst);
	if (ret < 0)
	{
		LogCritical("Failed to get interface list error: %s", strerror(errno));
		abort();
	}
	
	struct ifaddrs *tmp = iflst;
	if (tmp == NULL)
		return lst;
	while(tmp->ifa_next != NULL)
	{
		if ( (tmp->ifa_flags & IFF_UP) &&
		     (tmp->ifa_flags & IFF_LOOPBACK) == 0 &&
		     (tmp->ifa_flags & IFF_NOARP) == 0)
		{
			std::string ifname = tmp->ifa_name;
			lst.push_back(ifname);	
		}
		tmp = tmp->ifa_next;
	}
	freeifaddrs(iflst);
	return lst;
}

