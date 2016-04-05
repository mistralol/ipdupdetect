
#include <main.h>

static Mutex ExitLock;
static bool DoExit = false;

void print_help(FILE *fp, char *app)
{
	fprintf(fp, "Usage: %s <options>\n", app);
	fprintf(fp, "\n");
	fprintf(fp, " -h --help             Print this help\n");
	fprintf(fp, "\n");
}

class SigHandler : public ISignalHandler
{
	public:
		SigHandler() :
			m_manager(NULL)
		{

		}

		~SigHandler()
		{

		}
		
		void SetMonitorManager(MonitorManager *manager)
		{
			m_manager = manager;
		}

		void SigHUP(const siginfo_t *info)
		{
			LogDebug("SigHandler::SigHUP");
			LoggerRotate();
		}

		void SigTerm(const siginfo_t *info)
		{
			LogDebug("SigHandler::SigTerm");
			ScopedLock lock(&ExitLock);
			DoExit = true;
			ExitLock.WakeUp();
		}

		void SigUser1(const siginfo_t *info)
		{
			LogDebug("SigHandler::SigUser1");
			if (m_manager)
				m_manager->Dump();
		}

		void SigUser2(const siginfo_t *info)
		{
			LogDebug("SigHandler::SigUser2");
			if (m_manager)
				m_manager->Reset();
		}

		void SigPipe(const siginfo_t *info)
		{
			//Just ignored
		}

	private:
		MonitorManager *m_manager;
};


int main(int argc, char **argv)
{

	const char *opts = "h";
	int longindex = 0;
	int c = 0;
	struct option loptions[]
	{
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};
	
	while( (c = getopt_long(argc, argv, opts, loptions, &longindex)) >= 0)
	{
		switch(c)
		{
			case 'h':
				print_help(stdout, argv[0]);
				exit(EXIT_SUCCESS);
				break;
			default:
				break;
		}
	}
	
	//Add Logging
	if (isatty(fileno(stdout)) == 1)
	{
		LogManager::Add(new LogStdoutColor());
	}

	SigHandler SHandler;
	SignalHandler Signals = SignalHandler(&SHandler);
	Signals.Block();

	do
	{
		MonitorManager MManager;
		struct timespec timeout = {60, 0}; //Timeout to reprocess interface list
		ScopedLock lock(&ExitLock);
		SHandler.SetMonitorManager(&MManager);

		Signals.UnBlock();
		while(DoExit == false)
		{
			MManager.Scan();
			ExitLock.Wait(&timeout);
		}
		lock.Unlock(); //Required to prevent hang in signals
		
		Signals.Block();
		SHandler.SetMonitorManager(NULL);
		Signals.UnBlock();
		
	} while(0);


	return 0;
}


