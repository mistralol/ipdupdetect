
class Monitor : protected Thread
{
	public:
		Monitor(const std::string ifname);
		~Monitor();
	
		void Reset();
		void Dump();
	
	private:
		void Run();
		void Process(const struct pcap_pkthdr *hdr, const u_char *buf);
		void ProcessArp(const struct ether_arp *arp);
	
	private:
		std::string m_ifname;
		volatile bool m_exit;
		
		Mutex m_ipsmutex;
		std::map<std::string, std::string> m_ips;

};

