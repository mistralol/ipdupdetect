
#include <list>
#include <string>

#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include <liblogger.h>

using namespace liblogger;


#include <Interfaces.h>

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
		std::string ifname = tmp->ifa_name;
		lst.push_back(ifname);	
		tmp = tmp->ifa_next;
	}
	freeifaddrs(iflst);
	return lst;
}

