
#include <main.h>

MonitorManager::MonitorManager()
{

}

MonitorManager::~MonitorManager()
{
	RemoveAll();
}

void MonitorManager::Scan()
{
	ScopedLock lock(&m_mutex);
	LogDebug("MonitorManager::Scan()");
	std::list<std::string> iflst = Interfaces::List();
	std::map<std::string, bool> rmlst;
	
	for(auto it : m_map)
	{
		rmlst[it.first] = false;
	}

	for(auto it : iflst)
	{
		if (Exists(it) == false)
		{
			LogDebug("Adding Interface '%s'", it.c_str());
			Add(it);
		}
		else
		{
			rmlst[it] = true;
		}
	}
	
	for(auto it : rmlst)
	{
		if (it.second == false)
		{
			LogDebug("Removing Interface '%s'", it.first.c_str());
			Remove(it.first);
		}
	}
}

void MonitorManager::Purge()
{
	ScopedLock lock(&m_mutex);
	LogDebug("MonitorManager::Purge()");
	for(auto it : m_map)
	{
		it.second->Purge();
	}
}

void MonitorManager::Reset(const std::string ifname)
{
	ScopedLock lock(&m_mutex);
	auto it = m_map.find(ifname);
	if (it == m_map.end())
		abort(); //FIXME: Throw
	it->second->Reset();
}

void MonitorManager::Reset()
{
	ScopedLock lock(&m_mutex);
	for(auto it : m_map)
	{
		it.second->Reset();
	}
}

void MonitorManager::Dump(const std::string ifname)
{
	ScopedLock lock(&m_mutex);
	auto it = m_map.find(ifname);
	if (it == m_map.end())
		abort(); //FIXME: Throw
	it->second->Dump();
}

void MonitorManager::Dump()
{
	ScopedLock lock(&m_mutex);
	LogInfo("Dump info for all interfaces");
	for(auto it : m_map)
	{
		it.second->Dump();
	}
	LogInfo("End of dump");
}

void MonitorManager::Add(const std::string ifname)
{
	ScopedLock lock(&m_mutex);
	if (Exists(ifname))
		abort();
	std::shared_ptr<Monitor> ptr(new Monitor(ifname));
	m_map[ifname] = ptr;
}

bool MonitorManager::Exists(const std::string ifname)
{
	ScopedLock lock(&m_mutex);
	auto it = m_map.find(ifname);
	if (it == m_map.end())
		return false;
	return true;
}

void MonitorManager::Remove(const std::string ifname)
{
	ScopedLock lock(&m_mutex);
	auto it = m_map.find(ifname);
	m_map.erase(it);
}

void MonitorManager::RemoveAll()
{
	ScopedLock lock(&m_mutex);
	auto it = m_map.begin();
	while(it != m_map.end())
	{
		Remove(it->first);
		it = m_map.begin();
	}
}

