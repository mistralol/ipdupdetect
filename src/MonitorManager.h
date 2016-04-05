
class MonitorManager
{
	public:
		MonitorManager();
		~MonitorManager();

		void Scan();
		
		void Reset(const std::string ifname);
		void Reset();
		
		void Dump(const std::string ifname);
		void Dump();
		
	private:
		void Add(const std::string ifname);
		bool Exists(const std::string ifname);
		void Remove(const std::string ifname);
		void RemoveAll();
		
	
	private:
		Mutex m_mutex;
		std::map<std::string, std::shared_ptr<Monitor>> m_map;

};

